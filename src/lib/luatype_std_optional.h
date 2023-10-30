#ifndef LUATYPE_STD_OPTIONAL_H
#define LUATYPE_STD_OPTIONAL_H

#include "lua_templates.h"
#include <optional>

template<typename T>
struct LuaType<std::optional<T>> {
  static void pushdata(lua_State *L, std::optional<T> o) {
    if (o)
      LuaType<T>::pushdata(L, *o);
    else
      lua_pushnil(L);
  }

  static std::optional<T> &todata(lua_State *L, int i, C_State *C) {
    if (lua_type(L, i) == LUA_TNIL)
      return C->alloc<std::optional<T>>();
    else
      return C->alloc<std::optional<T>>(LuaType<T>::todata(L, i, C));
  }
};

#endif /* LUATYPE_STD_OPTIONAL_H */
