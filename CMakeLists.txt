cmake_minimum_required(VERSION 3.0.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(argp LANGUAGES CXX)

option(ARGP_TEST "Build and run tests" OFF)
option(ARGP_DOC "Build documentation" OFF)

add_library(argp INTERFACE)
target_include_directories(argp INTERFACE include)
target_compile_options(argp INTERFACE
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
