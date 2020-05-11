cmake_minimum_required (VERSION 3.16)
project (GameRemaker)

# Enable C++ 17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add src
aux_source_directory("${CMAKE_CURRENT_LIST_DIR}/src" SRC_FILES)
file(GLOB HEADER_FILES "${CMAKE_CURRENT_LIST_DIR}/src/*.h")
include_directories(GameRemaker "${CMAKE_CURRENT_LIST_DIR}/src")

# Sub dir
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/src/section")

# Executable
add_executable (GameRemaker ${HEADER_FILES} ${SRC_FILES})

# Add thirdparty lib
include_directories(GameRemaker "${CMAKE_CURRENT_LIST_DIR}/thirdparty/rapidjson/include")
include_directories(GameRemaker "${CMAKE_CURRENT_LIST_DIR}/thirdparty/tclap/include")
