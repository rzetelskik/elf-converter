#include "func.h"
#include <sstream>
#include <utility>
#include <fmt/core.h>

const size_t MAX_PARAMS = 6;

namespace {
    Func::PType parse(std::string &str) {
        if (str == "void") {
            return Func::pt_void;
        } else if (str == "int") {
            return Func::pt_int;
        } else if (str == "uint") {
            return Func::pt_uint;
        } else if (str == "long") {
            return Func::pt_long;
        } else if (str == "ulong") {
            return Func::pt_ulong;
        } else if (str == "longlong") {
            return Func::pt_longlong;
        } else if (str == "ulonglong") {
            return Func::pt_ulonglong;
        } else if (str == "ptr") {
            return Func::pt_ptr;
        } else {
            throw std::invalid_argument(fmt::format("invalid parameter type: {:s}", str));
        }
    }
}

std::unordered_map<std::string, Func> Func::load(std::ifstream &src) {
    std::unordered_map<std::string, Func> mp;

    std::string line;
    while (std::getline(src, line)) {
        std::istringstream iss(line);

        std::string name;
        if (!std::getline(iss, name, ' ')) {
            throw std::runtime_error("can't get function's name");
        }

        std::string param;
        if (!std::getline(iss, param, ' ')) {
            throw std::runtime_error("can't get function's return type");
        }
        Func::PType returnType = parse(param);

        std::vector<Func::PType> params;
        while (std::getline(iss, param, ' ')) {
            auto parsedParam = parse(param);
            if (parsedParam == pt_void) {
                throw std::runtime_error("function can't accept parameters of type void");
            }

            params.push_back(parsedParam);
        }

        if (params.size() > MAX_PARAMS) {
            throw std::runtime_error(
                    fmt::format("no more than 6 function parameter types are allowed, got: {:d}", params.size()));
        }

        mp.insert({name, Func(name, returnType, params)});
    }

    return mp;
}

Func::Func(std::string &name, Func::PType &returnType, std::vector<Func::PType> parameterTypes) :
        name(name), returnType(returnType), parameterTypes(std::move(parameterTypes)) {};

