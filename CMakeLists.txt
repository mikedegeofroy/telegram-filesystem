cmake_minimum_required(VERSION 3.5.1)

project(
    telegram-filesystem
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(PkgConfig REQUIRED)
pkg_search_module(FUSE REQUIRED fuse)

add_compile_options(-Wall -Weffc++ -pedantic -std=c++0x)

include_directories(${FUSE_INCLUDE_DIRS})

add_subdirectory(external/td)
add_subdirectory(src)
