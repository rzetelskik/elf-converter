#include "elf.h"
#include "symbol.h"

std::shared_ptr<Section> Elf::getSection(std::string const &name) {
    auto it = std::find_if(sections.begin(), sections.end(), [&](const auto &s) {
        return s->name == name;
    });

    if (it == sections.end()) {
        return nullptr;
    }

    return *it;
}

std::shared_ptr<RelocationSection> Elf::getRelocationSectionForSection(std::string const &refSecName) {
    auto it = std::find_if(relocationSections.begin(), relocationSections.end(), [&](const auto &rs) {
        return rs->refSec->name == refSecName;
    });

    if (it == relocationSections.end()) {
        return nullptr;
    }

    return *it;
}

std::shared_ptr<Symbol> Elf::getSectionSymbol(const std::string &name) {
    if (!symtabSection) {
        return nullptr;
    }

    return symtabSection->getSectionSymbol(name);
}
