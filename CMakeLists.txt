# CMakeLists.txt
cmake_minimum_required(VERSION 3.22)
set(PROJECT_NAME jlib)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)

project(${PROJECT_NAME})
message("CMake Build Type: ${CMAKE_BUILD_TYPE}")

include_directories(${CMAKE_HOME_DIRECTORY})
add_compile_definitions(ENABLE_TEST)

# file(GLOB_RECURSE test_files test/*.cpp)
file(GLOB_RECURSE test_files test/test.cpp
    test/test_swiss_vector.cpp
    # test/test_jenum.cpp
    # test/*.cpp
    # test/test_*.cpp
    # test/test_defer.cpp
)

add_executable(test_${PROJECT_NAME} ${test_files})
