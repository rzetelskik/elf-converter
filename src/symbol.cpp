#include "symbol.h"

Symbol::Symbol(std::string name, ELFIO::Elf64_Addr value, ELFIO::Elf_Xword size,
               unsigned char bind, unsigned char type, unsigned char other, std::shared_ptr<Section> section) :
        name(std::move(name)), value(value), size(size), bind(bind), type(type),
        other(other), section(std::move(section)), assignedIndex(std::nullopt) {};

bool Symbol::isGlobal() const {
    switch (bind) {
        case ELFIO::STB_GLOBAL:
        case ELFIO::STB_WEAK: // Not sure about this.
            return true;
        default:
            return false;
    }
}

bool Symbol::isExternal() const {
    return isGlobal() && section->type == ELFIO::SHT_NULL;
}

bool Symbol::isGlobalFunc() const {
    return isGlobal() && type == ELFIO::STT_FUNC;
}
