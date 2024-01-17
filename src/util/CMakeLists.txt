cmake_minimum_required(VERSION 3.10)
project(Util)

# setting the c++ settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# set macros for cleaner file
file(GLOB SOURCES ${CMAKE_SOURCE_DIR}/src/util/*.cpp)

add_library(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/dependencies/glm)
