set(RIME_SOURCE_DIR ${PROJECT_SOURCE_DIR})
project(rime-lua)
cmake_minimum_required(VERSION 2.8)
set(RIME_LUA_VERSION 1.0.0)
set(RIME_LUA_SOVERSION 1)

if(MSVC)
    include_directories(${RIME_SOURCE_DIR}/thirdparty/src/lua5.3)
    aux_source_directory(${RIME_SOURCE_DIR}/thirdparty/src/lua5.3 LUA_SRC)
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LUA REQUIRED luajit)
    include_directories(${LUA_INCLUDE_DIRS})
endif()

if(NOT DEFINED LIB_INSTALL_DIR)
    set(LIB_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib)
endif()

if(NOT DEFINED BIN_INSTALL_DIR)
    set(BIN_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/bin)
endif()

add_subdirectory(src)

set(rime_plugins_deps ${rime_plugins_deps} PARENT_SCOPE)
set(rime_default_modules ${rime_default_modules} PARENT_SCOPE)
