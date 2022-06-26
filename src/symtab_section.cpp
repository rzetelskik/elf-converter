#include "symtab_section.h"

SymtabSection::SymtabSection(std::vector<std::shared_ptr<Symbol>> &symbols)
        : Section(".symtab", ELFIO::SHT_SYMTAB, 0, 0x8, sizeof(ELFIO::Elf64_Sym)), symbols(symbols) {};

void SymtabSection::addSymbol(const std::shared_ptr<Symbol> &symbol) {
    symbols.push_back(symbol);
}

std::shared_ptr<Symbol> SymtabSection::getSectionSymbol(std::string const &_name) {
    auto it = std::find_if(symbols.begin(), symbols.end(), [&](const auto &s) {
        return _name == s->section->name && s->type == ELFIO::STT_SECTION;
    });

    if (it == symbols.end()) {
        return nullptr;
    }

    return *it;
}

void SymtabSection::rearrangeSymbols() {
    std::sort(symbols.begin(), symbols.end(), [](const auto &a, const auto &_) {
        return a->bind == ELFIO::STB_LOCAL;
    });
}

size_t SymtabSection::getFirstGlobalPosition() {
    auto firstGlobal = std::find_if(symbols.begin(), symbols.end(),
                                    [](const auto &a) { return a->bind == ELFIO::STB_GLOBAL; });

    return std::distance(symbols.begin(), firstGlobal);
}

