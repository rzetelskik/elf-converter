#ifndef CONVERTER_CONVERTER_H
#define CONVERTER_CONVERTER_H

#include "stub.h"
#include "elf.h"
#include "keystone/keystone.h"

class Converter {
public:
    Converter();

    ~Converter();

    void convert(Elf &elf, std::unordered_map<std::string, Func> &funcMap);

private:
    ks_engine *ks = nullptr;

    void generateStubs(Elf &elf, std::unordered_map<std::string, Func> &funcMap);

    std::optional<Stub> createStubForGlobalFuncSymbol(Elf &elf, std::unordered_map<std::string, Func> &funcMap,
                                                      std::shared_ptr<Symbol> &symbol);

    std::optional<Stub> createStubForExternalSymbol(Elf &elf, std::unordered_map<std::string, Func> &funcMap,
                                                    std::shared_ptr<Symbol> &symbol);

    void patchElfWithStub(Elf &elf, Stub &stub);

    std::vector<char> compile(std::string const &asmCode);
};


#endif //CONVERTER_CONVERTER_H
