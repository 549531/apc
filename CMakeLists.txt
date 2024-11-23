cmake_minimum_required(VERSION 3.20)

project(apc)

find_package(OpenCL REQUIRED)

add_executable(apc src/main.cpp)
target_link_libraries(apc OpenCL::OpenCL)
target_compile_definitions(apc PRIVATE CL_TARGET_OPENCL_VERSION=100)

install(TARGETS apc DESTINATION bin)
