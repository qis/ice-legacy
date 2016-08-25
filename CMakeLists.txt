cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(ice VERSION 0.1.0 LANGUAGES CXX)

# Settings
include(../common.cmake)

# Dependencies
find_package(compat REQUIRED PATHS ../compat)
find_package(libressl REQUIRED PATHS ../libressl)
find_package(zlib REQUIRED PATHS ../zlib)

# Sources
file(GLOB_RECURSE headers include/*.h)
assign_source_group(${headers})

file(GLOB_RECURSE sources src/*.h src/*.cpp)
assign_source_group(${sources})

# Library
add_library(${PROJECT_NAME} STATIC ${headers} ${sources})
target_link_libraries(${PROJECT_NAME} PUBLIC compat crypto zlib)

# Include Directories
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR} src PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

# Install Targets
if(CMAKE_BINARY_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
  if(MSVC)
    install(TARGETS ${PROJECT_NAME} DESTINATION lib/$<LOWER_CASE:$<CONFIG>>)
  else()
    install(TARGETS ${PROJECT_NAME} DESTINATION lib)
  endif()
endif()
