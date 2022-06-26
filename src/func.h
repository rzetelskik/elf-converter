#ifndef CONVERTER_FUNC_H
#define CONVERTER_FUNC_H

#include <vector>
#include <unordered_map>
#include <string>
#include <fstream>


class Func {
public:
    enum PType {
        pt_void,
        pt_int,
        pt_uint,
        pt_long,
        pt_ulong,
        pt_longlong,
        pt_ulonglong,
        pt_ptr
    };

    std::string name;
    PType returnType;
    std::vector<PType> parameterTypes;

    static std::unordered_map<std::string, Func> load(std::ifstream &src);

private:
    Func(std::string &name, PType &returnType, std::vector<PType> parameterTypes);
};


#endif