cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(ice VERSION 0.3.0 LANGUAGES C CXX)

# Common
include(../compat/compat.cmake)

# Packages
find_package(gsl REQUIRED PATHS "${CMAKE_CURRENT_LIST_DIR}/../gsl")
find_package(tls REQUIRED PATHS "${CMAKE_CURRENT_LIST_DIR}/../tls")

# Sources
file(GLOB include src)
file(GLOB_RECURSE headers include/*.h)
file(GLOB_RECURSE sources src/*.h src/*.cpp src/*.c)

# Library
assign_source_group(${headers} ${sources})
add_library(${PROJECT_NAME} STATIC ${headers} ${sources})
target_include_directories(${PROJECT_NAME} PUBLIC include PRIVATE src)
target_link_libraries(${PROJECT_NAME} PUBLIC gsl crypto)

# Install
if(CMAKE_BINARY_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
  if(MSVC)
    install(TARGETS ${PROJECT_NAME} DESTINATION lib/$<LOWER_CASE:$<CONFIG>>)
  else()
    install(TARGETS ${PROJECT_NAME} DESTINATION lib)
  endif()
endif()

# Format
if(CMAKE_BINARY_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
  add_custom_target(format
    COMMAND clang-format -i ${headers} ${sources}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()
