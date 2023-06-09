project(ArgueMeTest LANGUAGES CXX)

include(CTest)
include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.3.2
)

FetchContent_MakeAvailable(Catch2)

# list of all test targets as a dependency for a test launcher target
set(TEST_LIST)
set(TEST_DEPENDENCY ArgueMe)

macro(add_test_exec TEST_NAME)
    add_executable(${TEST_NAME} EXCLUDE_FROM_ALL ${ARGN})
    target_link_libraries(${TEST_NAME} PRIVATE
        ${TEST_DEPENDENCY}
        Catch2::Catch2WithMain)
    list(APPEND TEST_LIST ${TEST_NAME})
    add_test(NAME ${TEST_NAME}
        COMMAND ${TEST_NAME}
        --colour-mode ansi
    )
endmacro()

add_test_exec(SmokeTest smoke_test.cpp)
add_test_exec(ValueArg value_arg.cpp)
add_test_exec(MultiArg multi_arg.cpp)
add_test_exec(PosArg pos_arg.cpp)
add_test_exec(Command command.cpp)
add_test_exec(Prefix prefix.cpp)
add_test_exec(ParsingFlow parsing.cpp)

add_custom_target(MakeTest ALL
    ctest --output-on-failure --timeout 1 -V
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    USES_TERMINAL
    VERBATIM
    DEPENDS ${TEST_LIST}
)

enable_testing()
