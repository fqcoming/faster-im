
cmake_minimum_required(VERSION 3.0)
project(faster-im)

set(CMAKE_BUILD_TYPE "Debug")

# Set the path for project executable file output
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# Set the path for project library file output
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# Set the search path for project compilation header files: -I
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/pb)

# Set the search path for project library files: -L
link_directories(${PROJECT_SOURCE_DIR}/lib)

# src contains all the code for distributed IM projects
add_subdirectory(src)

