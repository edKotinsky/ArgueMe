cmake_minimum_required(VERSION 3.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(ArgueMe LANGUAGES CXX)

option(ARGUEME_TEST "Build and run tests" OFF)
option(ARGUEME_DOC "Build documentation" OFF)

add_library(ArgueMe INTERFACE )
target_include_directories(ArgueMe INTERFACE include)
target_compile_options(ArgueMe INTERFACE
    -Wall
    -Wpedantic
    -Wextra
)

if(ARGUEME_TEST)
    add_subdirectory(test)
endif()

if(ARGUEME_DOC)
    add_subdirectory(doc)
endif()
