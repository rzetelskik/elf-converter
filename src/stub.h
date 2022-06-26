#ifndef CONVERTER_STUB_H
#define CONVERTER_STUB_H

#include <memory>
#include "elf.h"
#include "section.h"

using StubSymbol = std::pair<const std::shared_ptr<Symbol> &, unsigned char>;

class Stub {
public:
    std::shared_ptr<Section> textSection = nullptr;
    std::shared_ptr<Symbol> textSectionSymbol = nullptr;
    std::shared_ptr<RelocationSection> textRelocationSection = nullptr;
    std::shared_ptr<Section> rodataSection = nullptr;
    std::shared_ptr<Symbol> rodataSectionSymbol = nullptr;
    std::shared_ptr<RelocationSection> rodataRelocationSection = nullptr;
    std::unordered_map<std::string, std::shared_ptr<Symbol>> symbolsReplacementsMap;

    Stub(std::istream &str, std::optional<StubSymbol> symbol);

    void appendCode(std::vector<char> const &bytes);

    void appendSuffix(const std::string &suffix);

    void mergeWith(const Stub &other);

    void patchWith(const Stub &other);

    std::shared_ptr<Symbol> getSymbolReplacement(std::string const &name);

private:
    void offsetSections(const Stub &other);

    void mergeSections(const Stub &other);

    void mergeRelocations(const Stub &other);

    void mergeSymbols(const Stub &other);

    void merge(const Stub &other);
};


#endif //CONVERTER_STUB_H
