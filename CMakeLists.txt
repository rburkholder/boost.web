cmake_minimum_required (VERSION 3.14)

PROJECT(boost.web.root)

# see https://blog.raymond.burkholder.net/index.php?/archives/1183-GCC-Optimization-for-Native-Architecture.html
add_definitions(-march=native)
add_definitions(-pthread)

if (POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(src)
