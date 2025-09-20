cmake_minimum_required(VERSION 3.10)
project(myGUI)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS})



add_executable(myGUI main.cpp)

target_include_directories(myGUI
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries(myGUI ${SDL2_LIBRARIES})

