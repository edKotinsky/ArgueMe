project(ArgueMeExamples LANGUAGES CXX)

add_executable(UsageExample usage.cpp)
target_link_libraries(UsageExample PUBLIC ArgueMe)
