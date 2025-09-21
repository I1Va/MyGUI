cmake_minimum_required(VERSION 3.10)
project(myGUI LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)

add_library(myGUI STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MyGUI.cpp
)

target_include_directories(myGUI
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries(myGUI PRIVATE SDL2::SDL2 SDL2_image::SDL2_image)
