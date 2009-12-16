project(TBO)

cmake_minimum_required(VERSION 2.8)

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

