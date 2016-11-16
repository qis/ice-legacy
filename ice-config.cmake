set(ICE_FOUND true)
set(ICE_VERSION "0.3.0")

if(NOT TARGET ice)
  find_package(gsl REQUIRED PATHS "${CMAKE_CURRENT_LIST_DIR}/../gsl")
  find_package(tls REQUIRED PATHS "${CMAKE_CURRENT_LIST_DIR}/../tls")
  add_library(ice STATIC IMPORTED)
  set_target_properties(ice PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/include"
    IMPORTED_LINK_INTERFACE_LANGUAGES "C;CXX"
    INTERFACE_LINK_LIBRARIES "gsl;crypto")
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
