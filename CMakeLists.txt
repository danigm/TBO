project(TBO)

cmake_minimum_required(VERSION 2.8)

ADD_SUBDIRECTORY(src)

INSTALL(PROGRAMS build/src/tbo DESTINATION bin)

#TODO, install data
