# CMakeLists.txt
cmake_minimum_required (VERSION 3.8.2)
project (goproWhereWhen)

set(CMAKE_SUPPRESS_REGENERATION true)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

file(GLOB SOURCES goproWhereWhen.cpp)
#file(GLOB SOURCES GPMF_parser.c "*.cpp" "GPMF_mp4reader.c")

add_executable(goproWhereWhen ${SOURCES})
