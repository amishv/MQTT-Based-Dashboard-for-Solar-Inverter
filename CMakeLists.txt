cmake_minimum_required(VERSION 3.0.0)
project(SolarDashboard VERSION 0.1.0)

#include(CTest)
#enable_testing()
# Use the package PkgConfig to detect GTK+ headers/library files
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
#Bring the headers,  into the project
include_directories(include ${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})
#Can manually add the sources using the set command as follows:
#set(SOURCES src/solarmon.c src/qpigs.c src/qpiri.c src/communication.c src/qmisc.c) -pedantic-Werror
#the file(GLOB...) allows for wildcard additions:
file(GLOB SOURCES "src/*.c")
add_executable(SolarDashboard ${SOURCES})
target_compile_options (SolarDashboard PRIVATE -Wall -Wextra -Werror -pedantic)
target_link_libraries (SolarDashboard paho-mqtt3c rt m ${GTK3_LIBRARIES})  