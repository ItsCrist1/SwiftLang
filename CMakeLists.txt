cmake_minimum_required(VERSION 3.31.11)
project(ShelLang)

set(CMAKE_CXX_STANDARD 23)

add_compile_options(-fsanitize=address -g)
add_link_options(-fsanitize=address)

# Include Files
include_directories(inc)

# Source Files
file(GLOB_RECURSE SOURCES src/*.cpp)

add_executable(ShelLang main.cpp ${SOURCES})