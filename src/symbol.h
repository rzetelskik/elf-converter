#ifndef CONVERTER_SYMBOL_H
#define CONVERTER_SYMBOL_H

#include <elfio/elfio.hpp>
#include <memory>
#include <utility>
#include "section.h"

class Symbol {
public:
    std::string name;
    ELFIO::Elf64_Addr value;
    ELFIO::Elf_Xword size;
    unsigned char bind;
    unsigned char type;
    unsigned char other;
    std::shared_ptr<Section> section;
    std::optional<ELFIO::Elf_Word> assignedIndex;

    Symbol(std::string name, ELFIO::Elf64_Addr value, ELFIO::Elf_Xword size,
           unsigned char bind, unsigned char type, unsigned char other, std::shared_ptr<Section> section);

    [[nodiscard]] bool isExternal() const;

    [[nodiscard]] bool isGlobalFunc() const;

private:
    [[nodiscard]] bool isGlobal() const;
};

#endif