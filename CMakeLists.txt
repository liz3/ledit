# idk just a small comment
cmake_minimum_required(VERSION 3.16)
project("textedit")
set(CMAKE_CXX_STANDARD 17)

add_executable(glfw-test glad.c la.cc lodepng.cpp main2.cc)
include_directories(/opt/homebrew/Cellar/freetype/2.11.0/include/freetype2)
target_link_directories(glfw-test PUBLIC /usr/local/lib /opt/homebrew/opt/freetype/lib)
include_directories(/opt/homebrew/Cellar/glfw/3.3.5/include/ /opt/homebrew/Cellar/glm/0.9.9.8/include)
target_link_directories(glfw-test PUBLIC /opt/homebrew/Cellar/glfw/3.3.5/lib/ /opt/homebrew/Cellar/glm/0.9.9.8/lib)
target_link_libraries(glfw-test PUBLIC glfw freetype)
