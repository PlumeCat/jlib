# CMakeLists.txt
cmake_minimum_required(VERSION 3.22)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang)
project(jlib)
message("CMake Build Type: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_FLAGS "-lstdc++ -lm")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_EXPORT_COMPILE_COMMANDS true)

include_directories(${CMAKE_HOME_DIRECTORY})
add_compile_definitions(ENABLE_TEST)

file(GLOB_RECURSE test_files test/*.cpp)

add_executable(test_jlib ${test_files})
add_test(NAME test COMMAND test_jlib)

# warnings
target_compile_options(test_jlib
    PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

