#ifndef CONVERTER_ELF_H
#define CONVERTER_ELF_H

#include <elfio/elfio.hpp>
#include "section.h"
#include "relocation_section.h"
#include <vector>
#include <memory>
#include "func.h"
#include "symtab_section.h"
#include "relocation.h"
#include "symbol.h"
#include "stub.h"


class Elf {
public:
    unsigned char elfClass;
    unsigned char osAbi;
    unsigned char encoding;
    ELFIO::Elf_Half fileType;
    ELFIO::Elf_Half machineNumber;

    std::vector<std::shared_ptr<Section>> sections;
    std::vector<std::shared_ptr<RelocationSection>> relocationSections;
    std::shared_ptr<SymtabSection> symtabSection;

    Elf(unsigned char elfClass, unsigned char osAbi, unsigned char encoding, ELFIO::Elf_Half fileType,
        ELFIO::Elf_Half machineNumber, std::vector<std::shared_ptr<Section>> sections,
        std::vector<std::shared_ptr<RelocationSection>> relocationSections,
        std::shared_ptr<SymtabSection> symtabSection) : elfClass(elfClass), osAbi(osAbi), encoding(encoding),
                                                        fileType(fileType), machineNumber(machineNumber),
                                                        sections(std::move(sections)),
                                                        relocationSections(std::move(relocationSections)),
                                                        symtabSection(std::move(symtabSection)) {};

    std::shared_ptr<Section> getSection(std::string const &name);

    std::shared_ptr<RelocationSection> getRelocationSectionForSection(std::string const &refSecName);

    std::shared_ptr<Symbol> getSectionSymbol(std::string const &name);
};


#endif