#ifndef CONVERTER_SYMTAB_SECTION_H
#define CONVERTER_SYMTAB_SECTION_H

#include <elfio/elfio.hpp>
#include "section.h"
#include "symbol.h"

class SymtabSection : public Section {
public:
    SymtabSection(std::vector<std::shared_ptr<Symbol>> &symbols);

    void addSymbol(std::shared_ptr<Symbol> const &symbol);
    std::shared_ptr<Symbol> getSectionSymbol(std::string const &name);
    void rearrangeSymbols();
    size_t getFirstGlobalPosition();

    std::vector<std::shared_ptr<Symbol>> symbols;
};

#endif