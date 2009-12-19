project(TBO)

cmake_minimum_required(VERSION 2.8)

set(DATA_DIR ${CMAKE_INSTALL_PREFIX}/share/tbo CACHE string "Shared directory")

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake 
${CMAKE_CURRENT_SOURCE_DIR}/config.h)

FIND_PACKAGE(GTK2)

set(SRCS
tbo.c
ui-drawing.c
ui-menu.c
ui-toolbar.c
)

INCLUDE_DIRECTORIES(${GTK2_INCLUDE_DIRS})

add_executable(tbo ${SRCS})

target_link_libraries(tbo ${GTK2_LIBRARIES})

