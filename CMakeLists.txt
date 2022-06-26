cmake_minimum_required(VERSION 3.18.4)
project(converter)

set(CMAKE_CXX_STANDARD 20)

set(SOURCES main.cpp
        src/elf.cpp src/elf.h
        src/func.cpp src/func.h
        src/section.cpp src/section.h
        src/symtab_section.cpp src/symtab_section.h
        src/symbol.cpp src/symbol.h
        src/relocation_section.cpp src/relocation_section.h
        src/relocation.h
        src/converter.cpp src/converter.h
        src/stub.cpp src/stub.h
        src/elfio_connector.cpp src/elfio_connector.h)

add_executable(converter ${SOURCES})

add_subdirectory(dependencies)

target_link_libraries(converter PRIVATE fmt elfio::elfio keystone)
target_include_directories(converter
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/keystone/include
        ${CMAKE_CURRENT_SOURCE_DIR}/static/include
        )
