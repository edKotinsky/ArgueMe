cmake_minimum_required(VERSION 3.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(argp LANGUAGES CXX)

option(ARGP_TEST "Build and run tests" OFF)
option(ARGP_DOC "Build documentation" OFF)

set(SOURCES source/parser.cpp)

add_library(argp STATIC ${SOURCES})
target_include_directories(argp PUBLIC include)
target_compile_options(argp PUBLIC
    -Wall
    -Wpedantic
    -Wextra
)

if(ARGP_TEST)
    add_subdirectory(test)
endif()

if(ARGP_DOC)
    add_subdirectory(doc)
endif()
