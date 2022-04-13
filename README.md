# cl

c(oncatinative/++) l(anguage) - Concatinative stack based prgramming language written in C++
it compiles down native executable and generates X86_64 assembly

## dependencies

1. Cmake >= 3.2
1. g++
1. make

## Compiling a program

```console
$ cmake --build ./build
$ ./build/cl c ./examples/test.cl
$ ./a.out       # native elf64 executable
```

## Simulating the program

```console
$ cmake --build ./build
$ ./build/cl s ./examples/test.cl
```

## Examples

cl is a stack based programming language, it uses postfix
notation/reverse polish notation in which operands comes
first and then operators/functions.

-   literal number is pushed onto stack
-   '+' expects two elements on the stack and pushes the sum.
-   '-' same as '+' but subracts them.
-   '.' prints the element in stack as integer.

Note: more features will be added

```code
3 4 + .
```
