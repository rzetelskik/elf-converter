#ifndef CONVERTER_SECTION_H
#define CONVERTER_SECTION_H

#include <elfio/elfio.hpp>
#include <optional>

class Section {
public:
    Section(std::string name, ELFIO::Elf_Word type, ELFIO::Elf_Xword flags);
    Section(std::string name, ELFIO::Elf_Word type, ELFIO::Elf_Xword flags, ELFIO::Elf_Xword addrAlign, ELFIO::Elf_Xword entrySize);
    Section(std::string name, ELFIO::Elf_Word type, ELFIO::Elf_Xword flags, ELFIO::Elf_Xword addrAlign, ELFIO::Elf_Xword entrySize, const char *rawData, ELFIO::Elf_Xword size);

    std::string name;
    ELFIO::Elf_Word type;
    ELFIO::Elf_Xword flags;
    ELFIO::Elf_Xword addrAlign;
    ELFIO::Elf_Xword entrySize;
    std::vector<char> data;

    std::optional<ELFIO::Elf_Half> assignedIndex = std::nullopt;
};

#endif