project(rime-lua)
cmake_minimum_required(VERSION 2.8)

set(RIME_LUA_VERSION 1.0.0)
set(RIME_LUA_SOVERSION 1)

option(BUILD_SHARED_LIBS "Build Rime as shared library" ON)
option(BUILD_STATIC "Build rime-lua using static libraries" OFF)
option(BUILD_TEST "Build and run tests" ON)

if(WIN32)
  message(FATAL_ERROR "Plugins are not supported under Windows.")
endif(WIN32)

set(RIME_SOURCE_DIR ${PROJECT_SOURCE_DIR}/..)
set(CMAKE_MODULE_PATH ${RIME_SOURCE_DIR}/cmake)

set(Gflags_STATIC ${BUILD_STATIC})
set(Glog_STATIC ${BUILD_STATIC})

find_package(Rime REQUIRED)
if(Rime_FOUND)
  include_directories(${Rime_INCLUDE_DIR})
endif(Rime_FOUND)

find_package(Gflags)
if(Gflags_FOUND)
  include_directories(${Gflags_INCLUDE_PATH})
endif(Gflags_FOUND)

find_package(Glog REQUIRED)
if(Glog_FOUND)
  include_directories(${Glog_INCLUDE_PATH})
endif(Glog_FOUND)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LUA REQUIRED luajit)
include_directories(${LUA_INCLUDE_DIRS})

include_directories(${RIME_SOURCE_DIR}/src)
include_directories(${RIME_SOURCE_DIR}/thirdparty/include)
link_directories(${RIME_SOURCE_DIR}/thirdparty/lib)
if(UNIX)
  add_definitions("-std=c++11")
endif(UNIX)

if(NOT DEFINED LIB_INSTALL_DIR)
    set(LIB_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib)
endif()

if(NOT DEFINED BIN_INSTALL_DIR)
    set(BIN_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/bin)
endif()

add_subdirectory(src)
