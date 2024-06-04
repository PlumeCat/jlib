# CMakeLists.txt
cmake_minimum_required(VERSION 3.22)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)

project(jlib)
message("CMake Build Type: ${CMAKE_BUILD_TYPE}")

include_directories(${CMAKE_HOME_DIRECTORY})
add_compile_definitions(ENABLE_TEST)

# file(GLOB_RECURSE test_files test/*.cpp)
file(GLOB_RECURSE test_files    test/*.cpp)

add_executable(test_jlib ${test_files})
target_compile_options(test_jlib PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX> $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)