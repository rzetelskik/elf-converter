#include "elfio_connector.h"
#include <fmt/core.h>


#define STRTAB_NAME ".strtab"

namespace {
    std::unordered_map<ELFIO::Elf_Half, std::shared_ptr<Section>> mapDataSections(ELFIO::elfio &reader) {
        std::unordered_map<ELFIO::Elf_Half, std::shared_ptr<Section>> sectionMap;

        for (auto &psec: reader.sections) {
            // Keep SHF_ALLOC and SHT_NULL. Discard notes. SHT_RELA sections will be collected separately.
            if (!(psec->get_flags() & ELFIO::SHF_ALLOC || psec->get_type() == ELFIO::SHT_NULL) ||
                 psec->get_type() == ELFIO::SHT_NOTE || psec->get_type() == ELFIO::SHT_RELA) {
                continue;
            }

            auto p = std::make_shared<Section>(psec->get_name(), psec->get_type(), psec->get_flags(),
                                               psec->get_addr_align(), psec->get_entry_size(), psec->get_data(),
                                               psec->get_size());
            sectionMap.insert({psec->get_index(), p});
        }

        return sectionMap;
    }

    std::unordered_map<unsigned int, std::shared_ptr<Symbol>>
    mapSymbols(ELFIO::elfio &reader, std::unordered_map<ELFIO::Elf_Half, std::shared_ptr<Section>> dataSectionsMap) {
        auto it = std::find_if(reader.sections.begin(), reader.sections.end(),
                               [](auto &psec) { return psec->get_type() == ELFIO::SHT_SYMTAB; });

        if (it == reader.sections.end()) {
            return {};
        }
        auto symSec = *it;

        std::unordered_map<unsigned int, std::shared_ptr<Symbol>> symbolMap;

        ELFIO::symbol_section_accessor symbolSectionAccessor(reader, symSec);
        // The symbol table entry for index 0 (STN_UNDEF) is reserved and holds (0,0,0,0,0,0,SHN_UNDEF). ELFIO creates it itself, so we just skip.
        for (unsigned int i = 1; i < symbolSectionAccessor.get_symbols_num(); i++) {
            std::string name;
            ELFIO::Elf64_Addr value;
            ELFIO::Elf_Xword size;
            unsigned char bind;
            unsigned char type;
            ELFIO::Elf_Half sectionIndex;
            unsigned char other;

            bool ret = symbolSectionAccessor.get_symbol(i, name, value, size, bind, type, sectionIndex, other);
            if (!ret) {
                throw std::runtime_error(fmt::format("can't retrieve symbol at index {:d}", i));
            }

            if (type == ELFIO::STT_FILE) {
                // Ignore symbols of type STT_FILE.
                continue;
            }

            if (name.empty() && type == ELFIO::STT_SECTION) {
                name = reader.sections[sectionIndex]->get_name();
            }

            auto section = dataSectionsMap.find(sectionIndex);
            if (section == dataSectionsMap.end()) {
                // Ignore symbols referring filtered sections.
                continue;
            }

            auto p = std::make_shared<Symbol>(name, value, size, bind, type, other, section->second);
            symbolMap.insert({i, p});
        }

        return symbolMap;
    }

    std::vector<std::shared_ptr<RelocationSection>> getRelocationSections(ELFIO::elfio &reader,
                                                                          std::unordered_map<ELFIO::Elf_Half, std::shared_ptr<Section>> dataSectionsMap,
                                                                          std::unordered_map<unsigned int, std::shared_ptr<Symbol>> symbolMap) {
        std::vector<std::shared_ptr<RelocationSection>> relocationSections;

        for (const auto &psec: reader.sections) {
            if (psec->get_type() != ELFIO::SHT_RELA) {
                continue;
            }

            auto sectionIt = dataSectionsMap.find(psec->get_info());
            if (sectionIt == dataSectionsMap.end()) {
                throw std::runtime_error(
                        fmt::format("can't get section referenced by SHT_RELA section {:s} at index {:d}",
                                    psec->get_name(),
                                    psec->get_info()));
            }

            std::vector<Relocation> relocations;
            ELFIO::relocation_section_accessor relocationSectionAccessor(reader, psec);
            for (unsigned int i = 0; i < relocationSectionAccessor.get_entries_num(); i++) {
                ELFIO::Elf64_Addr offset;
                ELFIO::Elf_Word symbolIndex;
                unsigned char type;
                ELFIO::Elf_Sxword addend;

                bool ret = relocationSectionAccessor.get_entry(i, offset, symbolIndex, type, addend);
                if (!ret) {
                    throw std::runtime_error(
                            fmt::format("can't get relocation in section {:s} at index {:d}", psec->get_name(), i));
                }

                auto symbolIt = symbolMap.find(symbolIndex);
                if (symbolIt == symbolMap.end()) {
                    throw std::runtime_error(
                            fmt::format("can't find symbol at index {:d} referenced by relocation {:d} in section {:s}",
                                        symbolIndex, i, psec->get_name()));
                }

                relocations.emplace_back(offset, type, addend, symbolIt->second);
            }

            auto p = std::make_shared<RelocationSection>(sectionIt->second, relocations);
            relocationSections.push_back(p);
        }

        return relocationSections;
    }

    ELFIO::section *createDataSection(ELFIO::elfio &writer, std::shared_ptr<Section> const &section) {
        auto psec = writer.sections.add(section->name);
        psec->set_type(section->type);
        psec->set_flags(section->flags);
        psec->set_addr_align(section->addrAlign);
        psec->set_entry_size(section->entrySize);
        psec->set_data(section->data.data(), section->data.size());
        section->assignedIndex = psec->get_index();

        return psec;
    }

    ELFIO::section *createStrtab(ELFIO::elfio &writer) {
        auto psec = writer.sections.add(STRTAB_NAME);
        psec->set_type(ELFIO::SHT_STRTAB);
        psec->set_addr_align(0x1); // The value 0 or 1 means that the section has no alignment constraints.
        psec->set_flags(0x0);

        return psec;
    }

    ELFIO::section *createSymtab(ELFIO::elfio &writer, const std::shared_ptr<SymtabSection> &symtabSection,
                                 ELFIO::section *strSec) {
        auto psec = writer.sections.add(symtabSection->name);
        psec->set_type(symtabSection->type);
        psec->set_addr_align(symtabSection->addrAlign);
        psec->set_entry_size(writer.get_default_entry_size(ELFIO::SHT_SYMTAB));
        psec->set_link(strSec->get_index());
        // Add 1 for NULL symbol added by ELFIO on creation.
        psec->set_info(symtabSection->getFirstGlobalPosition() + 1);
        psec->set_flags(symtabSection->flags);

        ELFIO::string_section_accessor stringSectionAccessor(strSec);
        ELFIO::symbol_section_accessor symbolSectionAccessor(writer, psec);

        for (auto &sym: symtabSection->symbols) {
            auto symIndex = symbolSectionAccessor.add_symbol(stringSectionAccessor, sym->name.c_str(), sym->value,
                                                             sym->size, sym->bind, sym->type, sym->other,
                                                             sym->section->assignedIndex.value());
            sym->assignedIndex = symIndex;
        }

        return psec;
    }

    ELFIO::section *createRelocationSection(ELFIO::elfio &writer,
                                            std::shared_ptr<RelocationSection> const &relocationSection,
                                            ELFIO::Elf_Half symtabIndex) {
        auto psec = writer.sections.add(relocationSection->name);
        psec->set_type(relocationSection->type);
        psec->set_info(relocationSection->refSec->assignedIndex.value());
        psec->set_link(symtabIndex);
        psec->set_flags(relocationSection->flags);

        psec->set_addr_align(relocationSection->addrAlign);
        psec->set_entry_size(writer.get_default_entry_size(relocationSection->type));

        ELFIO::relocation_section_accessor relocationSectionAccessor(writer, psec);
        for (auto &rel: relocationSection->relocations) {
            if (relocationSection->type == ELFIO::SHT_REL) {
                relocationSectionAccessor.add_entry(rel.offset, rel.symbol->assignedIndex.value(), rel.type);
            } else {
                relocationSectionAccessor.add_entry(rel.offset, rel.symbol->assignedIndex.value(), rel.type,
                                                    rel.addend);
            }
        }

        return psec;
    }
}

void ElfioConnector::validateRel64LE(ELFIO::elfio &reader) {
    auto err = reader.validate();
    if (!err.empty()) {
        throw std::runtime_error(fmt::format("invalid ELF source file: {:s}", err));
    }

    if (reader.get_class() != ELFIO::ELFCLASS64) {
        throw std::runtime_error("unsupported ELF file class!");
    }

    if (reader.get_type() != ELFIO::ET_REL) {
        throw std::runtime_error("unsupported ELF file type!");
    }

    if (reader.get_encoding() != ELFIO::ELFDATA2LSB) {
        throw std::runtime_error("unsupported ELF file format encoding");
    }

    if (reader.get_machine() != ELFIO::EM_X86_64) {
        throw std::runtime_error("unsupported ELF file architecture");
    }
}

Elf ElfioConnector::read(ELFIO::elfio &reader) {
    auto elfClass = reader.get_class();
    auto osAbi = reader.get_os_abi();
    auto encoding = reader.get_encoding();
    auto fileType = reader.get_type();
    auto machineNumber = reader.get_machine();

    auto dataSectionsMap = mapDataSections(reader);
    auto symbolMap = mapSymbols(reader, dataSectionsMap);
    auto relocationSections = getRelocationSections(reader, dataSectionsMap, symbolMap);

    auto getSecond = [](const auto &p) { return p.second; };

    std::vector<std::shared_ptr<Section>> sections;
    std::transform(dataSectionsMap.begin(), dataSectionsMap.end(), std::back_inserter(sections), getSecond);

    std::vector<std::shared_ptr<Symbol>> symbols;
    std::transform(symbolMap.begin(), symbolMap.end(), std::back_inserter(symbols), getSecond);

    auto symtabSection = std::make_shared<SymtabSection>(symbols);

    return {elfClass, osAbi, encoding, fileType, machineNumber, sections, relocationSections, symtabSection};
}

void ElfioConnector::write(ELFIO::elfio &writer, Elf &elf) {
    writer.create(elf.elfClass, elf.encoding);
    writer.set_os_abi(elf.osAbi);
    writer.set_type(elf.fileType);
    writer.set_machine(elf.machineNumber);

    for (auto &sec: elf.sections) {
        // ELFIO always creates SHT_NULL section at index 0. Update the index only.
        if (sec->type == ELFIO::SHT_NULL) {
            sec->assignedIndex = (ELFIO::Elf_Half) 0x0;
            continue;
        }

        createDataSection(writer, sec);
    }

    auto strSec = createStrtab(writer);

    elf.symtabSection->rearrangeSymbols();
    auto symSec = createSymtab(writer, elf.symtabSection, strSec);

    for (auto &relSec: elf.relocationSections) {
        createRelocationSection(writer, relSec, symSec->get_index());
    }
}


