#ifndef CONVERTER_ELFIO_CONNECTOR_H
#define CONVERTER_ELFIO_CONNECTOR_H

#include "elf.h"
#include <elfio/elfio.hpp>

class ElfioConnector {
public:
    static void validateRel64LE(ELFIO::elfio &reader);

    static Elf read(ELFIO::elfio &reader);

    static void write(ELFIO::elfio &writer, Elf &elf);
};


#endif //CONVERTER_ELFIO_CONNECTOR_H
