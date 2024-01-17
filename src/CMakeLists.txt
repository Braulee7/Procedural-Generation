cmake_minimum_required(VERSION 3.10)
project(Main)

# setting the c++ settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# set macros for cleaner file
file(GLOB SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp)

# add utility subdirectory
add_subdirectory(util)

# dependencies
find_package(Vulkan REQUIRED COMPONENTS glslc)
find_program(glslc_executable NAMES glslc HINTS Vulkan::glslc)
if (NOT Vulkan_FOUND)
	message(FATAL_ERROR "Couldn't find the vulkan SDK")
endif()

set(DEPENDENT_LIBRARIES ${Vulkan_LIBRARIES}
                        glfw
                        Util)
set(DEPENDENT_INCLUDE ${Vulkan_INCLUDE_DIRS}
                      glfw 
                      ${CMAKE_SOURCE_DIR}/dependencies/glm
                      ${CMAKE_SOURCE_DIR}/src/util)

# COMPILE SHADERS
set(SHADER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
set(SHADER_BINARY_DIR ${CMAKE_BINARY_DIR}/shaders)

file(GLOB SHADERS
	${SHADER_SOURCE_DIR}/*.vert
	${SHADER_SOURCE_DIR}/*.frag)

add_custom_command(
  COMMAND
    ${CMAKE_COMMAND} -E make_directory ${SHADER_BINARY_DIR}
  OUTPUT ${SHADER_BINARY_DIR}
  COMMENT "Creating ${SHADER_BINARY_DIR}"
)

foreach(source IN LISTS SHADERS)
  get_filename_component(FILENAME ${source} NAME)
  add_custom_command(
    COMMAND
      ${glslc_executable}
      #      -MD -MF ${SHADER_BINARY_DIR}/${FILENAME}.d
      -o ${SHADER_BINARY_DIR}/${FILENAME}.spv
      ${source}
    OUTPUT ${SHADER_BINARY_DIR}/${FILENAME}.spv
    DEPENDS ${source} ${SHADER_BINARY_DIR}
    COMMENT "Compiling ${FILENAME}"
  )
  list(APPEND SPV_SHADERS ${SHADER_BINARY_DIR}/${FILENAME}.spv)
endforeach()

add_custom_target(shaders ALL DEPENDS ${SPV_SHADERS})


# the entry point of the program
add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC ${DEPENDENT_INCLUDE})
target_link_libraries(${PROJECT_NAME} PUBLIC ${DEPENDENT_LIBRARIES})
