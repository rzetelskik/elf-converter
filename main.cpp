#include <fstream>
#include <vector>
#include <elfio/elfio.hpp>
#include <fmt/core.h>
#include "src/func.h"
#include "src/converter.h"
#include "src/elfio_connector.h"

int main(int argc, char *argv[]) {
    if (argc != 4) {
        std::cerr << fmt::format("Usage: {:s} <source file> <function list file> <destination file>", argv[0]) << std::endl;
        return 1;
    }

    try {
        std::ifstream src(argv[1], std::fstream::binary);
        if (!src.is_open()) {
            throw std::runtime_error("unable to open source file");
        }

        std::ifstream flist(argv[2], std::fstream::binary);
        if (!flist.is_open()) {
            throw std::runtime_error("unable to open function list file");
        }

        ELFIO::elfio reader;
        if (!reader.load(src)) {
            throw std::runtime_error("can't process source file");
        }
        ElfioConnector::validateRel64LE(reader);

        auto elf = ElfioConnector::read(reader);

        auto funcMap = Func::load(flist);

        Converter converter;
        converter.convert(elf, funcMap);

        ELFIO::elfio writer;
        ElfioConnector::write(writer, elf);
        auto err = writer.validate();
        if (!err.empty()) {
            throw std::runtime_error("output elf is invalid");
        }

        writer.save(argv[3]);
    } catch (std::exception &e) {
        fmt::print(stderr,"FATAL: {:s}\n", e.what());
        return 1;
    }

    return 0;
}