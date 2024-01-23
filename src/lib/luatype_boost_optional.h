#ifndef LUATYPE_BOOST_OPTIONAL_H
#define LUATYPE_BOOST_OPTIONAL_H

#include "lua_templates.h"
#include <boost/optional.hpp>

template<typename T>
struct LuaType<boost::optional<T>> {
  static void pushdata(lua_State *L, boost::optional<T> o) {
    if (o)
      LuaType<T>::pushdata(L, *o);
    else
      lua_pushnil(L);
  }

  static boost::optional<T> &todata(lua_State *L, int i, C_State *C) {
    if (lua_type(L, i) == LUA_TNIL)
      return C->alloc<boost::optional<T>>();
    else
      return C->alloc<boost::optional<T>>(LuaType<T>::todata(L, i, C));
  }
};

#endif /* LUATYPE_BOOST_OPTIONAL_H */
