# CMakeLists.txt
cmake_minimum_required (VERSION 3.8.2)
project (goproWhereWhen)

set(CMAKE_SUPPRESS_REGENERATION true)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

# Warnings as errors too...
# SET(GCC_COVERAGE_COMPILE_FLAGS "-Wall -Wextra -Wpedantic -Werror -std=c++11")
SET(GCC_COVERAGE_COMPILE_FLAGS "-Wall -Wextra -Wpedantic -std=c++11")
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")

# SET(GCC_COVERAGE_LINK_FLAGS    "-lgcov")
# SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")

file(GLOB SOURCES goproWhereWhen.cpp opts.cpp utils.cpp)
#file(GLOB SOURCES GPMF_parser.c "*.cpp" "GPMF_mp4reader.c")

add_executable(goproWhereWhen ${SOURCES})
