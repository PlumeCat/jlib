# CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
set(PROJECT_NAME jlib)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)

project(${PROJECT_NAME})
message("CMake Build Type: ${CMAKE_BUILD_TYPE}")

include_directories(${CMAKE_HOME_DIRECTORY})
add_compile_definitions(ENABLE_TEST)

file(GLOB_RECURSE source_files test/*.cpp)

add_executable(${PROJECT_NAME} ${source_files})
