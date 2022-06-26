#ifndef CONVERTER_RELOCATION_H
#define CONVERTER_RELOCATION_H

#include <elfio/elfio.hpp>
#include <memory>
#include "symbol.h"

class Relocation {
public:
    ELFIO::Elf64_Addr offset;
    unsigned char type;
    ELFIO::Elf_Sxword addend;
    std::shared_ptr<Symbol> symbol;

    Relocation(ELFIO::Elf64_Addr offset, unsigned char type, ELFIO::Elf_Sxword addend, std::shared_ptr<Symbol> &symbol)
            : offset(offset), type(type), addend(addend), symbol(symbol) {};
};

#endif