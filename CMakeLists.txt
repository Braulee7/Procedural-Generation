cmake_minimum_required(VERSION 3.10)
project(Procedural_Generation VERSION 1.0 LANGUAGES CXX)

# disable the glfw tests
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(src)
add_subdirectory(dependencies/glm)
add_subdirectory(dependencies/glfw)