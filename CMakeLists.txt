
# CMake minimum version
cmake_minimum_required(VERSION 3.13)

# HIFAL project.
project(hifal VERSION 1.0.0.0)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add source files.
#set(SOURCES src/main.c src/hifal.c src/mimes.c src/socketBuffer.c src/web.c)

# Add source files (using GLOB).
file(GLOB_RECURSE SOURCES src/*.c)

# Include headers.
include_directories(headers)

# Build program.
ADD_EXECUTABLE(hifal  ${SOURCES})


