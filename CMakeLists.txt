cmake_minimum_required(VERSION 3.16)

project(demo)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 99)

find_package(Freetype REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 3.3 REQUIRED)

include_directories(${GLEW_INCLUDE_DIRS} ${FREETYPE_INCLUDE_DIRS} include)

add_executable(demo
   demo.cpp
   lib/gllabel.cpp
   lib/types.cpp
   lib/vgrid.cpp
   lib/cubic2quad.cpp
   lib/outline.cpp
)

target_link_libraries(demo Freetype::Freetype GLEW::GLEW glfw)
