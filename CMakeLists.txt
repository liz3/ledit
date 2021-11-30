# idk just a small comment
cmake_minimum_required(VERSION 3.16)
project("ledit")
set(CMAKE_CXX_STANDARD 17)

add_executable(ledit src/glad.c src/la.cc src/main.cc)
include_directories(/opt/homebrew/Cellar/freetype/2.11.0/include/freetype2 /usr/local/Cellar/glfw/3.3.5/include /usr/local/opt/freetype/include/freetype2)
target_link_directories(ledit PUBLIC /usr/local/lib /opt/homebrew/opt/freetype/lib)
include_directories(/opt/homebrew/Cellar/glfw/3.3.5/include/ /opt/homebrew/Cellar/glm/0.9.9.8/include)
target_link_directories(ledit PUBLIC /opt/homebrew/Cellar/glfw/3.3.5/lib/ /usr/local/Cellar/glfw/3.3.5/lib /usr/local/opt/freetype/lib)
target_link_libraries(ledit PUBLIC glfw freetype)
file(COPY fonts DESTINATION .)
