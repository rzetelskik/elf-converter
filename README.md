# Binary files converter

A 64 to 32 bit ELF ET_REL file converter.

## Assignment

This project was created as a solution to the [1st assignment](https://students.mimuw.edu.pl/ZSO/PUBLIC-SO/2021-2022/z1_elf/index.html) for the "Advanced topics in operating systems" course at the University of Warsaw.

## Build

To build the executable file run the below command:
```
mkdir build && cmake -B build && cd build && make && cp converter ../converter
```
An executable file `converter` should now be in the project's root directory.

## Other
`static` directory contains source files used for creating precompiled stubs.
