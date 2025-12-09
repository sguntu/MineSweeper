cmake_minimum_required(VERSION 3.28)
project(min12 LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.2
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)

add_executable(min12 main.cpp
        GameWindow.cpp
        GameWindow.h
        Minesweeper.cpp
        Minesweeper.h)
target_compile_features(min12 PRIVATE cxx_std_17)
target_link_libraries(min12 PRIVATE SFML::Graphics)
