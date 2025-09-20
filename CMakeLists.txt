cmake_minimum_required(VERSION 3.10)
project(myGUI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SDL2 and SDL2_image
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)

# Add executable
add_executable(myGUI main.cpp)

# Include your own headers
target_include_directories(myGUI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)

# Link SDL2 and SDL2_image using modern targets
target_link_libraries(myGUI PRIVATE SDL2::SDL2 SDL2_image::SDL2_image)
