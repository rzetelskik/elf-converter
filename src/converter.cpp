#include <ranges>
#include <sstream>
#include <numeric>
#include <functional>
#include <fmt/core.h>

#include "converter.h"
#include "keystone/keystone.h"
#include "stub.h"
#include "stubs.h"

namespace {
    void convertSymtab(std::shared_ptr<SymtabSection> const &symtabSection) {
        symtabSection->addrAlign = 0x4;
        symtabSection->entrySize = sizeof(ELFIO::Elf32_Sym);
    }

    std::string getOpForPType64(Func::PType &p) {
        switch (p) {
            case Func::PType::pt_int:
            case Func::PType::pt_uint:
            case Func::PType::pt_ptr:
            case Func::PType::pt_ulong:
                return "movl";
            case Func::PType::pt_long:
                return "movslq";
            case Func::PType::pt_longlong:
            case Func::PType::pt_ulonglong:
                return "movq";
            default:
                throw std::invalid_argument("can't get size of invalid parameter type");
        }
    }

    std::string getOpForPType32(Func::PType &p) {
        switch (p) {
            case Func::PType::pt_int:
            case Func::PType::pt_uint:
            case Func::PType::pt_ptr:
            case Func::PType::pt_ulong:
            case Func::PType::pt_long:
                return "movl";
            case Func::PType::pt_longlong:
            case Func::PType::pt_ulonglong:
                return "movq";
            default:
                throw std::invalid_argument("can't get size of invalid parameter type");
        }
    }

    std::string getRegister(size_t i, bool isQWord) {
        static const std::string DWRegisters[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
        static const std::string QWRegisters[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

        return (isQWord ? QWRegisters : DWRegisters)[i];

    }

    std::string getRegisterForPType64(Func::PType &p, size_t i) {
        switch (p) {
            case Func::PType::pt_int:
            case Func::PType::pt_uint:
            case Func::PType::pt_ptr:
            case Func::PType::pt_ulong:
                return getRegister(i, false);
            case Func::PType::pt_long:
            case Func::PType::pt_longlong:
            case Func::PType::pt_ulonglong:
                return getRegister(i, true);
            default:
                throw std::invalid_argument("can't get register for invalid parameter type");
        }
    }

    std::string getRegisterForPType32(Func::PType &p, size_t i) {
        switch (p) {
            case Func::PType::pt_int:
            case Func::PType::pt_uint:
            case Func::PType::pt_ptr:
            case Func::PType::pt_ulong:
            case Func::PType::pt_long:
                return getRegister(i, false);
            case Func::PType::pt_longlong:
            case Func::PType::pt_ulonglong:
                return getRegister(i, true);
            default:
                throw std::invalid_argument("can't get register for invalid parameter type");
        }
    }

    size_t getPTypeSize64(Func::PType &p) {
        switch (p) {
            case Func::PType::pt_int:
            case Func::PType::pt_uint:
                return 0x4;
            case Func::PType::pt_ptr:
            case Func::PType::pt_ulong:
            case Func::PType::pt_long:
            case Func::PType::pt_longlong:
            case Func::PType::pt_ulonglong:
                return 0x8;
            default:
                throw std::invalid_argument("can't get size of invalid parameter type");
        }
    }

    size_t getPTypeSize32(Func::PType &p) {
        switch (p) {
            case Func::PType::pt_int:
            case Func::PType::pt_uint:
            case Func::PType::pt_ptr:
            case Func::PType::pt_ulong:
            case Func::PType::pt_long:
                return 0x4;
            case Func::PType::pt_longlong:
            case Func::PType::pt_ulonglong:
                return 0x8;
            default:
                throw std::invalid_argument("can't get size of invalid parameter type");
        }
    }

    std::string convertFuncParameters64(std::vector<Func::PType> &parameters) {
        size_t offset = 0x10;

        std::stringstream instructions;
        for (size_t i = 0; i < parameters.size(); i++) {
            auto p = parameters[i];
            auto op = getOpForPType64(p);
            auto reg = getRegisterForPType64(p, i);

            instructions << fmt::format("{:s} {:#04x}(%rsp), %{:s};", op, offset, reg);
            offset += getPTypeSize64(p);
        }

        return instructions.str();
    }

    size_t getAlignedStackSize32(size_t offset) {
        while (offset % 16 != 8) {
            offset += 4;
        }
        return offset;
    }

    std::string convertFuncParameters32(std::vector<Func::PType> &parameters) {
        size_t offset = 0;

        std::vector<std::string> rinstructions;
        for (size_t i = 0; i < parameters.size(); i++) {
            auto p = parameters[i];
            auto op = getOpForPType32(p);
            auto reg = getRegisterForPType32(p, i);
            rinstructions.push_back(fmt::format("{:s} %{:s}, {:#04x}(%rsp);", op, reg, offset));

            offset += getPTypeSize32(p);
        }
        rinstructions.push_back(fmt::format("subq ${:#04x}, %rsp;", getAlignedStackSize32(offset)));

        std::stringstream instructions;
        for (auto &i: std::ranges::reverse_view(rinstructions)) {
            instructions << i;
        }

        return instructions.str();
    }

    std::string convertFuncRetType32(Func::PType &p) {
        switch (p) {
            case Func::PType::pt_longlong:
            case Func::PType::pt_ulonglong:
                return "mov %eax, %eax;"
                       "shlq $0x20, %rdx;"
                       "orq %rdx, %rax;";
            case Func::PType::pt_long:
                return "cdqe;";
            default:
                return "";
        }
    }

    std::string convertFunc32Return(Func::PType &ret, std::vector<Func::PType> &parameters) {
        auto size = std::accumulate(parameters.begin(), parameters.end(), 0, [&](size_t size, auto &p) {
            return size + getPTypeSize32(p);
        });

        return convertFuncRetType32(ret) + fmt::format("addq ${:#04x}, %rsp;", getAlignedStackSize32(size));
    }

    std::optional<Stub> foldSymbols(std::vector<std::shared_ptr<Symbol>> &symbols, auto &&f) {
        auto reduce = [&](std::optional<Stub> a, std::shared_ptr<Symbol> &sym) {
            auto b = f(sym);

            if (!a.has_value()) {
                return b;
            }
            if (b.has_value()) {
                a->mergeWith(b.value());
            }

            return a;
        };

        return std::accumulate(symbols.begin(), symbols.end(), static_cast<std::optional<Stub>>(std::nullopt),
                               reduce);
    }

    unsigned char convertRelocationType(unsigned char type) {
        switch (type) {
            case ELFIO::R_X86_64_32:
            case ELFIO::R_X86_64_32S:
                return ELFIO::R_386_32;
            case ELFIO::R_X86_64_PC32:
            case ELFIO::R_X86_64_PLT32:
                return ELFIO::R_386_PC32;
            default:
                throw std::invalid_argument("can't convert unsupported relocation type");
        }
    }

    void convertRelocation(Relocation &rel, std::shared_ptr<Section> const &refSec) {
        rel.type = convertRelocationType(rel.type);

        auto _addend = htole32(static_cast<uint32_t>(rel.addend));
        const char *src = static_cast<const char *>(static_cast<void *>(&_addend));
        std::copy(src, src + sizeof(uint32_t), refSec->data.begin() + (uint32_t) rel.offset);
    }


    void convertRelocationSection(std::shared_ptr<RelocationSection> &rs) {
        rs->type = ELFIO::SHT_REL;
        // Update name using the referenced section's name.
        rs->name = fmt::format(".rel{:s}", rs->refSec->name);

        for (auto &rel: rs->relocations) {
            convertRelocation(rel, rs->refSec);
        }
    }
}


Converter::Converter() {
    ks_err err = ks_open(KS_ARCH_X86, KS_MODE_64, &ks);
    if (err != KS_ERR_OK) {
        throw std::runtime_error("can't ks_open");
    }

    ks_option(ks, KS_OPT_SYNTAX, KS_OPT_SYNTAX_ATT);
}

void Converter::convert(Elf &elf, std::unordered_map<std::string, Func> &funcMap) {
    // Convert class and architecture.
    elf.elfClass = ELFIO::ELFCLASS32;
    elf.machineNumber = ELFIO::EM_386;

    convertSymtab(elf.symtabSection);

    generateStubs(elf, funcMap);

    // Convert relocation sections.
    for (auto &rs: elf.relocationSections) {
        convertRelocationSection(rs);
    }
}

void Converter::generateStubs(Elf &elf, std::unordered_map<std::string, Func> &funcMap) {
    std::vector<std::shared_ptr<Symbol>> globalFuncSymbols;
    std::copy_if(elf.symtabSection->symbols.begin(), elf.symtabSection->symbols.end(), std::back_inserter(globalFuncSymbols), [](const auto &s) {
        return s->isGlobalFunc();
    });

    std::vector<std::shared_ptr<Symbol>> externalSymbols;
    std::copy_if(elf.symtabSection->symbols.begin(), elf.symtabSection->symbols.end(), std::back_inserter(externalSymbols), [](const auto &s) {
        return s->isExternal();
    });

    auto thunkin = foldSymbols(globalFuncSymbols,
                               std::bind(&Converter::createStubForGlobalFuncSymbol, this, elf, funcMap,
                                         std::placeholders::_1));
    if (thunkin.has_value()) {
        thunkin->appendSuffix(".thunkin");
        patchElfWithStub(elf, thunkin.value());
    }

    auto thunkout = foldSymbols(externalSymbols,
                                std::bind(&Converter::createStubForExternalSymbol, this, elf, funcMap,
                                          std::placeholders::_1));
    if (thunkout.has_value()) {
        thunkout->appendSuffix(".thunkout");
        patchElfWithStub(elf, thunkout.value());
    }
}

// createStubForGlobalFuncSymbol creates a stub that allows for calling a 64bit function from 32bit code.
std::optional<Stub> Converter::createStubForGlobalFuncSymbol(Elf &elf, std::unordered_map<std::string, Func> &funcMap,
                                                             std::shared_ptr<Symbol> &symbol) {
    // Find corresponding func in map.
    auto funcIt = funcMap.find(symbol->name);
    if (funcIt == funcMap.end()) {
        // If symbol is not in funcMap, treat it like data symbol and skip conversion.
        return std::nullopt;
    }
    auto func = funcIt->second;

    // Change existing symbol's bind to local.
    // Existing symbol will be called from stub.
    symbol->bind = ELFIO::STB_LOCAL;

    // Create elf transferring from 32 to 64 bits.
    std::istringstream _32to64(
            std::string(reinterpret_cast<const char *>(__64from32_32to64_o), __64from32_32to64_o_len));
    Stub stub32to64(_32to64, std::nullopt);
    stub32to64.appendCode(compile(convertFuncParameters64(func.parameterTypes)));

    // Create elf calling the 64 bit func and then reverting to 32 bits.
    std::istringstream call64Restore32(
            std::string(reinterpret_cast<const char *>(__64from32_call64restore32_o), __64from32_call64restore32_o_len));
    Stub stubCall64Restore32(call64Restore32, std::make_pair(symbol, ELFIO::STB_GLOBAL));

    auto stub = stub32to64;
    stub.patchWith(stubCall64Restore32);

    return stub;
}

std::optional<Stub> Converter::createStubForExternalSymbol(Elf &elf, std::unordered_map<std::string, Func> &funcMap,
                                                           std::shared_ptr<Symbol> &symbol) {
    // Find corresponding func in map.
    auto funcIt = funcMap.find(symbol->name);
    if (funcIt == funcMap.end()) {
        // If symbol is not in funcMap, treat it like data symbol and skip conversion.
        return std::nullopt;
    }
    auto func = funcIt->second;

    std::istringstream save64(
            std::string(reinterpret_cast<const char *>(__32from64_save64_o), __32from64_save64_o_len));
    Stub stubSave64(save64, std::nullopt);
    auto asmcode = convertFuncParameters32(func.parameterTypes);
    stubSave64.appendCode(compile(convertFuncParameters32(func.parameterTypes)));

    std::istringstream call32(
            std::string(reinterpret_cast<const char *>(__32from64_call32_o), __32from64_call32_o_len));
    Stub stubCall32(call32, std::make_pair(symbol, ELFIO::STB_LOCAL));
    stubCall32.appendCode(compile(convertFunc32Return(func.returnType, func.parameterTypes)));

    std::istringstream restore64(
            std::string(reinterpret_cast<const char *>(__32from64_restore64_o), __32from64_restore64_o_len));
    Stub stubRestore64(restore64, std::nullopt);

    auto stub = stubSave64;
    stub.patchWith(stubCall32);
    stub.patchWith(stubRestore64);

    auto replacementSymbol = stub.getSymbolReplacement(symbol->name);
    if (!replacementSymbol) {
        throw std::runtime_error("can't get replacement symbol from generated stub");
    }

    for (auto &rs: elf.relocationSections) {
        for (auto &r: rs->relocations) {
            if (r.symbol == symbol) {
                r.symbol = replacementSymbol;
            }
        }
    }

    return stub;
}

void Converter::patchElfWithStub(Elf &elf, Stub &stub) {
    elf.sections.push_back(stub.textSection);
    elf.relocationSections.push_back(stub.textRelocationSection);
    elf.symtabSection->addSymbol(stub.textSectionSymbol);
    elf.sections.push_back(stub.rodataSection);
    elf.relocationSections.push_back(stub.rodataRelocationSection);
    elf.symtabSection->addSymbol(stub.rodataSectionSymbol);
    for (auto &p: stub.symbolsReplacementsMap) {
        elf.symtabSection->addSymbol(p.second);
    }
}

// compile compiles assembly instructions. WARNING: instructions need to be separated by ';' or '\n'
std::vector<char> Converter::compile(std::string const &asmCode) {
    unsigned char *encode;
    size_t size;
    size_t _count;

    if (ks_asm(ks, asmCode.c_str(), 0, &encode, &size, &_count) != KS_ERR_OK) {
        throw std::runtime_error(fmt::format("can't compile asm code, err code: {:d}", ks_errno(ks)));
    }

    std::vector<char> bytes(encode, encode + size);
    ks_free(encode);

    return bytes;
}


Converter::~Converter() {
    ks_close(ks);
}

