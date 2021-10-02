cmake_minimum_required(VERSION 3.0)
project(zynmidicontroller)

include(CheckIncludeFiles)
include(CheckLibraryExists)

link_directories(/usr/local/lib)

add_library(zynmidicontroller SHARED zynmidicontroller.h zynmidicontroller.cpp)
	target_link_libraries(zynmidicontroller jack lo)

install(TARGETS zynmidicontroller LIBRARY DESTINATION lib)
