cmake_minimum_required(VERSION 3.20)

project(apc)

include(cmake/CPM.cmake)

CPMAddPackage(
  NAME SDL2
  VERSION 2.30.1
  GITHUB_REPOSITORY libsdl-org/SDL
  GIT_TAG release-2.30.1)

add_executable(apc src/main.cpp)
target_link_libraries(apc SDL2::SDL2)

install(TARGETS apc DESTINATION bin)
