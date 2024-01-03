cmake_minimum_required(VERSION 3.10)
project(Main)

# setting the c++ settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# set macros for cleaner file
file(GLOB SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp)

# dependencies
find_package(Vulkan REQUIRED)
if (NOT Vulkan_FOUND)
	message(FATAL_ERROR "Couldn't find the vulkan SDK")
endif()

set(DEPENDENT_LIBRARIES ${Vulkan_LIBRARIES} glfw)
set(DEPENDENT_INCLUDE ${Vulkan_INCLUDE_DIRS} glfw ${CMAKE_SOURCE_DIR}/dependencies/glm)

# the entry point of the program
add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC ${DEPENDENT_INCLUDE})
target_link_libraries(${PROJECT_NAME} PUBLIC ${DEPENDENT_LIBRARIES})
