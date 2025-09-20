set(LUA_VERSION "lua" CACHE STRING "lua version")
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/lua5.4/lua.h")
  find_package(PkgConfig)
  if(PkgConfig_FOUND)
    foreach(pkg ${LUA_VERSION} lua54 lua53 lua52 luajit lua51)
      pkg_check_modules(LUA IMPORTED_TARGET GLOBAL ${pkg})
      if(LUA_FOUND)
	break()
      endif()
    endforeach()
  endif()
  if(LUA_FOUND)
    set(LUA_TARGET PkgConfig::LUA)
    include_directories(${LUA_INCLUDE_DIRS})
  else()
    message(FATAL_ERROR "Lua not found, consider using `bash action-install.sh` to download.")
  endif()
else()
  message(STATUS "Using in-tree lua source")
  include_directories(thirdparty/lua5.4)
  aux_source_directory(thirdparty/lua5.4 LUA_SRC)
  add_definitions(-DLUA_COMPAT_5_3)
  if(NOT WIN32)
    set(USE_INTREE_LUA ON)
    set_property(SOURCE ${LUA_SRC} PROPERTY COMPILE_DEFINITIONS LUA_USE_POSIX;LUA_USE_DLOPEN)
    execute_process(
      COMMAND ${CMAKE_C_COMPILER} -print-multiarch
      OUTPUT_VARIABLE multiarch
      ERROR_QUIET)
    if(multiarch)
      string(REGEX REPLACE "[\r\n]" "" multiarch ${multiarch})
      add_definitions(-DLUA_MULTIARCH=\"${multiarch}\")
      message(STATUS "with multiarch: ${multiarch}")
    endif()
  endif()
endif()

aux_source_directory(src RIME_LUA_SRC)
aux_source_directory(src/lib RIME_LUA_LIB_SRC)

add_library(rime-lua-objs OBJECT ${RIME_LUA_SRC} ${RIME_LUA_LIB_SRC} ${LUA_SRC})
if(BUILD_SHARED_LIBS)
  set_target_properties(rime-lua-objs PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

set(plugin_name "rime-lua" PARENT_SCOPE)
set(plugin_objs $<TARGET_OBJECTS:rime-lua-objs> PARENT_SCOPE)
set(plugin_deps ${LUA_TARGET} ${rime_library} ${rime_gears_library} PARENT_SCOPE)
set(plugin_modules "lua" PARENT_SCOPE)

# Hide Lua symbols on Unix-like systems when using in-tree Lua
# for external lua engine capability
if (USE_INTREE_LUA)
  # On Unix-like systems, hide Lua symbols using compiler attributes
  # Apply to object files only to avoid affecting main library
  set_target_properties(rime-lua-objs PROPERTIES
    C_VISIBILITY_PRESET hidden
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
  )
  # Apply hidden visibility to Lua source files specifically
  set_source_files_properties(${LUA_SRC} PROPERTIES COMPILE_FLAGS "-fvisibility=hidden")
endif(USE_INTREE_LUA)