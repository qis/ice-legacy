set(ICE_FOUND true)
set(ICE_VERSION "0.1.0")

find_package(compat REQUIRED PATHS "${CMAKE_CURRENT_LIST_DIR}/../compat")
find_package(libressl REQUIRED PATHS "${CMAKE_CURRENT_LIST_DIR}/../libressl")

if(NOT TARGET ice)
  add_library(ice STATIC IMPORTED)
  set_target_properties(ice PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/include"
    IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
    INTERFACE_LINK_LIBRARIES "compat;crypto")
  if(MSVC)
    set_target_properties(ice PROPERTIES
      IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_LIST_DIR}/lib/debug/ice.lib"
      IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_LIST_DIR}/lib/release/ice.lib"
      IMPORTED_CONFIGURATIONS "DEBUG;RELEASE")
  else()
    set_target_properties(ice PROPERTIES
      IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/lib/libice.a"
      IMPORTED_CONFIGURATIONS "RELEASE")
  endif()
endif()
