cmake_minimum_required(VERSION 3.10)
project(MyGUI LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)

add_library(MyGUI STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MyGUI.cpp
)

target_include_directories(MyGUI
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries(MyGUI 
                      PRIVATE SDL2::SDL2 SDL2_image::SDL2_image
                      PRIVATE SDL2_ttf::SDL2_ttf
                      PUBLIC geometry_module)
