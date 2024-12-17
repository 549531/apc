cmake_minimum_required(VERSION 3.20)

project(apc)

include(cmake/CPM.cmake)

CPMAddPackage(
  NAME SDL2
  VERSION 2.30.1
  GITHUB_REPOSITORY libsdl-org/SDL
  GIT_TAG release-2.30.1)

CPMAddPackage(
  NAME incbin
  GIT_REPOSITORY https://github.com/graphitemaster/incbin
  GIT_TAG main)

find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(apc src/main.cpp)
target_link_libraries(apc SDL2::SDL2main SDL2::SDL2 GLEW::GLEW  OpenGL::GL OpenGL::GLU)
target_include_directories(apc PRIVATE ${incbin_SOURCE_DIR})

install(TARGETS apc DESTINATION bin)
