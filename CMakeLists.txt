cmake_minimum_required (VERSION 4.0)

# CMakeLists for tools
project(uRWellTestProto VERSION 1.0.1 DESCRIPTION "Tools for analysing data from uRwell Test prototype")

# Set up
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules")
set(CMAKE_PREFIX_PATH $ENV{ROOTSYS}/cmake)
#set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/build")
find_package(ROOT REQUIRED COMPONENTS RIO Net)
find_package(XYHodoTools REQUIRED)
find_package(LZ4 REQUIRED)
add_definitions(-D__LZ4__)
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

include_directories(${PROJECT_SOURCE_DIR}/include ${ROOT_INCLUDE_DIRS})
set(uRWell_SOURCE_FILES
  "${PROJECT_SOURCE_DIR}/src/uRwellTools.cc" )
set(uRWell_HEADERS
  "${PROJECT_SOURCE_DIR}/include/uRwellTools.h" )
add_library(uRWellLib SHARED ${uRWell_SOURCE_FILES})
set_target_properties(uRWellLib PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 1)
set (CMAKE_INSTALL_LIBDIR "lib")
#target_include_directories(uRWellLib PRIVATE include)
target_include_directories(uRWellLib
  PUBLIC
  $<INSTALL_INTERFACE:include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  )

include(GNUInstallDirs)
#include(CMakePackageConfigHelpers)

install(TARGETS uRWellLib
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

configure_file(uRWellLib.pc.in uRWellLib.pc @ONLY)
  
install(FILES ${CMAKE_BINARY_DIR}/uRWellLib.pc DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)

message("ROOT_INCLUDE_DIR is " ${ROOT_INCLUDE_DIRS})

# Add directories
add_subdirectory(hipo/hipo4)
#add_subdirectory(hipo)
#find_package(hipo4 REQUIRED)
add_subdirectory(AnaCodes)
add_subdirectory(EventViewer)
add_subdirectory(misc/HVScans)
