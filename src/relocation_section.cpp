#include "relocation_section.h"
#include <fmt/core.h>

RelocationSection::RelocationSection(std::shared_ptr<Section> &_refSec,
                                     std::vector<Relocation> relocations) :
        Section(fmt::format(".rela{:s}", _refSec->name), ELFIO::SHT_RELA, ELFIO::SHF_INFO_LINK, 0x8,
                sizeof(ELFIO::Elf64_Rela)), relocations(std::move(relocations)) {
    if (!_refSec) {
        throw std::invalid_argument("nullptr provided as a referenced section to relocation section");
    }
    refSec = _refSec;
};