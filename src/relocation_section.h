#ifndef CONVERTER_RELOCATION_SECTION_H
#define CONVERTER_RELOCATION_SECTION_H

#include <elfio/elfio.hpp>
#include "section.h"
#include "relocation.h"
#include <memory>

class RelocationSection : public Section {
public:
    std::shared_ptr<Section> refSec = nullptr;
    std::vector<Relocation> relocations;

    RelocationSection(std::shared_ptr<Section> &refSec, std::vector<Relocation> relocations);
};

#endif