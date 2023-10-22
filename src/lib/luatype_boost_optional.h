#ifndef LUATYPE_BOOST_OPTIONAL_H
#define LUATYPE_BOOST_OPTIONAL_H

#include "lua_templates.h"
#if __cplusplus >= 201703L || _MSVC_LANG >= 201703L
#include <optional>
namespace ns = std;
#else
#include <boost/optional.hpp>
namespace ns = boost;
#endif

template<typename T>
struct LuaType<ns::optional<T>> {
  static void pushdata(lua_State *L, ns::optional<T> o) {
    if (o)
      LuaType<T>::pushdata(L, *o);
    else
      lua_pushnil(L);
  }

  static ns::optional<T> &todata(lua_State *L, int i, C_State *C) {
    if (lua_type(L, i) == LUA_TNIL)
      return C->alloc<ns::optional<T>>();
    else
      return C->alloc<ns::optional<T>>(LuaType<T>::todata(L, i, C));
  }
};

#endif /* LUATYPE_BOOST_OPTIONAL_H */
