cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(ice VERSION 0.2.0 LANGUAGES CXX)

# Compiler
set(CMAKE_CXX_FLAGS "-std=c++1z -Wall ${CMAKE_CXX_FLAGS}")

if(CMAKE_BUILD_TYPE MATCHES "Debug")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -Werror -Wno-unused-parameter -Wno-unused-variable")
  #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined -fno-omit-frame-pointer")
endif()

# Dependencies
find_package(compat REQUIRED ../compat)
find_package(OpenSSL REQUIRED)

# Sources
file(GLOB_RECURSE headers include/*.h)
file(GLOB_RECURSE sources src/*.h src/*.cpp)

# Library
add_library(${PROJECT_NAME} STATIC ${headers} ${sources})
target_include_directories(${PROJECT_NAME} PUBLIC include PRIVATE src ${OPENSSL_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC compat ${OPENSSL_LIBRARIES})

# Format
add_custom_target(format
  COMMAND clang-format -i ${headers} ${sources}
  COMMAND perl -pi -0777 -e "s/{\\\\n[ \\\\t]*}/{}/g\; s/\\\\r//g" ${headers} ${sources}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# Install
install(TARGETS ${PROJECT_NAME} DESTINATION lib)
