#include "stub.h"
#include "elfio_connector.h"
#include <fmt/core.h>


#define TEXT_SECTION_NAME ".text"
#define RODATA_SECTION_NAME ".rodata"
#define HARDCODED_SYMBOL_NAME "fun"

Stub::Stub(std::istream &str, std::optional<StubSymbol> symbol) {
    // Read a precompiled asm code and parse it to elf structure.
    ELFIO::elfio reader;

    if (!reader.load(str)) {
        throw std::runtime_error("elfio reader can't load istream");
    }

    auto err = reader.validate();
    if (!err.empty()) {
        throw std::runtime_error(fmt::format("invalid elf source file, err: {:s}", err));
    }

    auto elf = ElfioConnector::read(reader);

    textSection = elf.getSection(TEXT_SECTION_NAME);
    if (!textSection) {
        throw std::runtime_error("can't get section .text");
    }

    textRelocationSection = elf.getRelocationSectionForSection(TEXT_SECTION_NAME);
    if (!textRelocationSection) {
        textRelocationSection = std::make_shared<RelocationSection>(textSection, std::vector<Relocation>());
    }

    textSectionSymbol = elf.getSectionSymbol(TEXT_SECTION_NAME);
    if (!textSectionSymbol) {
        textSectionSymbol = std::make_shared<Symbol>(TEXT_SECTION_NAME, 0, 0, ELFIO::STB_LOCAL, ELFIO::STT_SECTION, ELFIO::STV_DEFAULT, textSection);
    }

    rodataSection = elf.getSection(RODATA_SECTION_NAME);
    if (!rodataSection) {
        rodataSection = std::make_shared<Section>(RODATA_SECTION_NAME, ELFIO::SHT_PROGBITS, ELFIO::SHF_ALLOC);
    }

    rodataRelocationSection = elf.getRelocationSectionForSection(RODATA_SECTION_NAME);
    if (!rodataRelocationSection) {
        rodataRelocationSection = std::make_shared<RelocationSection>(rodataSection, std::vector<Relocation>());
    }

    rodataSectionSymbol = elf.getSectionSymbol(RODATA_SECTION_NAME);
    if (!rodataSectionSymbol) {
        rodataSectionSymbol = std::make_shared<Symbol>(RODATA_SECTION_NAME, 0, 0, ELFIO::STB_LOCAL, ELFIO::STT_SECTION, ELFIO::STV_DEFAULT, rodataSection);
    }

    if (symbol.has_value()) {
        // st_value specifies the position in section
        // st_size specifies the size of function's code, can be omitted if we only care about the address
        auto replacementSymbol = std::make_shared<Symbol>(symbol->first->name, 0, textSection->data.size(), symbol->second, ELFIO::STT_FUNC, ELFIO::STV_DEFAULT, textSection);
        symbolsReplacementsMap.insert({symbol->first->name, replacementSymbol});


        // Replace hardcoded symbol from precompiled code with replacement symbol.
        for (auto &r: textRelocationSection->relocations) {
            if (r.symbol->name == HARDCODED_SYMBOL_NAME) {
                r.symbol = symbol.value().first;
            }
        }
    }
}

// mergeWith merges stubs created for multiple symbols
void Stub::mergeWith(Stub const &other) {
    for (auto &s: other.symbolsReplacementsMap) {
        // Adjust st_value (position in section).
        s.second->value += textSection->data.size();
    }

    merge(other);
}

// patchWith patches sections of stub create for singular symbol
void Stub::patchWith(Stub const &other) {
    for (auto &s: other.symbolsReplacementsMap) {
        // Adjust st_size (size of symbol/section).
        s.second->size += textSection->data.size();
    }

    merge(other);
}

void Stub::merge(Stub const &other) {
    mergeSymbols(other);
    offsetSections(other);
    mergeSections(other);
    mergeRelocations(other);
}

void Stub::mergeSymbols(Stub const &other) {
    for (auto &s: other.symbolsReplacementsMap) {
        s.second->section = textSection;
    }

    symbolsReplacementsMap.insert(other.symbolsReplacementsMap.begin(), other.symbolsReplacementsMap.end());
}

void Stub::offsetSections(Stub const &other) {
    for (auto &r: other.textRelocationSection->relocations) {
        if (r.symbol->section->name == RODATA_SECTION_NAME) {
            r.symbol = rodataSectionSymbol;
            r.addend += rodataSection->data.size();
        }
        r.offset += textSection->data.size();
    }

    for (auto &r: other.rodataRelocationSection->relocations) {
        if (r.symbol->section->name == TEXT_SECTION_NAME) {
            r.symbol = textSectionSymbol;
            r.addend += textSection->data.size();
        }
        r.offset += rodataSection->data.size();
    }
}

void Stub::mergeSections(Stub const &other) {
    textSection->data.insert(textSection->data.end(), other.textSection->data.begin(), other.textSection->data.end());
    rodataSection->data.insert(rodataSection->data.end(), other.rodataSection->data.begin(), other.rodataSection->data.end());
}

void Stub::mergeRelocations(Stub const &other) {
    textRelocationSection->relocations.insert(textRelocationSection->relocations.end(), other.textRelocationSection->relocations.begin(), other.textRelocationSection->relocations.end());
    rodataRelocationSection->relocations.insert(rodataRelocationSection->relocations.end(), other.rodataRelocationSection->relocations.begin(), other.rodataRelocationSection->relocations.end());
}

void Stub::appendCode(const std::vector<char> &bytes) {
    textSection->data.insert(textSection->data.end(), bytes.begin(), bytes.end());
}

void Stub::appendSuffix(const std::string &suffix) {
    textSection->name = textSection->name + suffix;
    textSectionSymbol->name = textSectionSymbol->name + suffix;
    rodataSection->name = rodataSection->name + suffix;
    rodataSectionSymbol->name = rodataSectionSymbol->name + suffix;
}

std::shared_ptr<Symbol> Stub::getSymbolReplacement(const std::string &name) {
    auto it = symbolsReplacementsMap.find(name);

    if (it == symbolsReplacementsMap.end()) {
        return nullptr;
    }

    return it->second;
}
