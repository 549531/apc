cmake_minimum_required(VERSION 3.20)

project(apc)

include(cmake/CPM.cmake)

CPMAddPackage(
  NAME SDL2
  VERSION 2.30.1
  GITHUB_REPOSITORY libsdl-org/SDL
  GIT_TAG release-2.30.1)

find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(apc src/main.cpp)
target_compile_features(apc PRIVATE cxx_std_20)

if(SDL2_SDL2main_FOUND)
  target_link_libraries(apc SDL2::SDL2main)
endif()

target_link_libraries(apc SDL2::SDL2 GLEW::GLEW OpenGL::GL OpenGL::GLU)

install(TARGETS apc DESTINATION bin)
