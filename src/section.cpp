#include "section.h"

#include <utility>

// sh_addralign value 0 or 1 means that the section has no alignment constraints.
Section::Section(std::string name, ELFIO::Elf_Word type, ELFIO::Elf_Xword flags) : Section(std::move(name), type, flags, 0, 0, nullptr, 0) {};

Section::Section(std::string name, ELFIO::Elf_Word type, ELFIO::Elf_Xword flags, ELFIO::Elf_Xword addrAlign,
                 ELFIO::Elf_Xword entrySize) : Section(std::move(name), type, flags, addrAlign, entrySize, nullptr, 0) {};

Section::Section(std::string name, ELFIO::Elf_Word type, ELFIO::Elf_Xword flags, ELFIO::Elf_Xword addrAlign, ELFIO::Elf_Xword entrySize, const char *rawData, ELFIO::Elf_Xword size):
        name(std::move(name)), type(type), flags(flags), addrAlign(addrAlign), entrySize(entrySize) {
    if (rawData) data = std::vector(rawData, rawData + size);
};
