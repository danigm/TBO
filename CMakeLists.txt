project(TBO)

cmake_minimum_required(VERSION 2.8)

ADD_SUBDIRECTORY(src)

INSTALL(PROGRAMS ${CMAKE_BINARY_DIR}/src/tbo DESTINATION bin)
INSTALL(DIRECTORY tbo-data DESTINATION ${CMAKE_INSTALL_PREFIX}/share)

# == UNINSTALL == 

CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
