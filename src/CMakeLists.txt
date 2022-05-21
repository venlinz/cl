cmake_minimum_required(VERSION 3.2)

project(cl VERSION 0.1
    LANGUAGES CXX)

add_compile_options(-Wall -Wextra -pedantic -Werror
    -pedantic-errors -Wconversion -Wshadow -ggdb3
    -std=c++20)
add_executable(cl main.cpp main.h)
