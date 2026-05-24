cmake_minimum_required(VERSION 3.31.11)
project(ShelLang)

set(CMAKE_CXX_STANDARD 23)

add_compile_options(-fsanitize=address -g)
add_link_options(-fsanitize=address)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include Files
include_directories(inc)

# Source Files
file(GLOB_RECURSE SOURCES src/*.cpp)

add_executable(ShelLang main.cpp ${SOURCES})

# Tests (Google Test via FetchContent)
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.15.2
)
FetchContent_MakeAvailable(googletest)

enable_testing()

file(GLOB_RECURSE TEST_SOURCES tests/*.cpp tests/*.h)

add_executable(ShelLangTests ${TEST_SOURCES} ${SOURCES})
target_link_libraries(ShelLangTests PRIVATE GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(ShelLangTests)
