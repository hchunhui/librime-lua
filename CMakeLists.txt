set(RIME_SOURCE_DIR ${PROJECT_SOURCE_DIR})
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

add_subdirectory(src)

set(rime_plugins_deps ${rime_plugins_deps} PARENT_SCOPE)
set(rime_default_modules ${rime_default_modules} PARENT_SCOPE)
