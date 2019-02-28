#include "lua.h"
#include "lua_templates.h"

namespace rime {

namespace LuaImpl {
  static int index(lua_State *L) {
    if (luaL_getmetafield(L, 1, "methods") != LUA_TNIL) {
      lua_pushvalue(L, 2);
      lua_rawget(L, -2);
      if (!lua_isnil(L, -1)) {
        return 1;
      } else {
        lua_pop(L, 1);
      }
    }

    if (luaL_getmetafield(L, 1, "vars_get") != LUA_TNIL) {
      lua_pushvalue(L, 2);
      lua_rawget(L, -2);
      if (!lua_isnil(L, -1)) {
        auto f = lua_tocfunction(L, -1);
        lua_pop(L, 1);
        if (f) {
          lua_remove(L, 2);
          return f(L);
        }
      }
    }
    return 0;
  }

  static int newindex(lua_State *L) {
    if (luaL_getmetafield(L, 1, "vars_set") != LUA_TNIL) {
      lua_pushvalue(L, 2);
      lua_rawget(L, -2);
      if (!lua_isnil(L, -1)) {
        auto f = lua_tocfunction(L, -1);
        lua_pop(L, 1);
        if (f) {
          lua_remove(L, 2);
          return f(L);
        }
      }
    }
    return 0;
  }

  void export_type(lua_State *L,
                   const char *name, lua_CFunction gc,
                   const luaL_Reg *funcs, const luaL_Reg *methods,
                   const luaL_Reg *vars_get, const luaL_Reg *vars_set) {
    for (int i = 0; funcs[i].name; i++) {
      lua_register(L, funcs[i].name, funcs[i].func);
    }

    luaL_newmetatable(L, name);
    if (gc) {
      lua_pushstring(L, "__gc");
      lua_pushcfunction(L, gc);
      lua_settable(L, -3);
    }
    lua_pushstring(L, "methods");
    lua_createtable(L, 0, 4);
    luaL_setfuncs(L, methods, 0);
    lua_settable(L, -3);
    lua_pushstring(L, "vars_get");
    lua_createtable(L, 0, 4);
    luaL_setfuncs(L, vars_get, 0);
    lua_settable(L, -3);
    lua_pushstring(L, "vars_set");
    lua_createtable(L, 0, 4);
    luaL_setfuncs(L, vars_set, 0);
    lua_settable(L, -3);
    lua_pushstring(L, "__index");
    lua_pushcfunction(L, index);
    lua_settable(L, -3);
    lua_pushstring(L, "__newindex");
    lua_pushcfunction(L, newindex);
    lua_settable(L, -3);
    lua_pop(L, 1);
  }

  static int yield(lua_State *L) {
    return lua_yield(L, lua_gettop(L));
  }

  extern "C" void xluaopen_utf8(lua_State *);
  void types_init(lua_State *L);

  static const char makeclosurekey = 'k';
  static const char luakey = 'k';

  static int pmain(lua_State *L) {
    luaL_openlibs(L);
    xluaopen_utf8(L);
    lua_register(L, "yield", yield);
    types_init(L);

    int status = luaL_dofile(L, (string(RimeGetUserDataDir()) + "/rime.lua").c_str());
    if (status != LUA_OK) {
      const char *e = lua_tostring(L, -1);
      printf("dofile(err=%d): %s\n", status, e);
    }

    lua_pushlightuserdata(L, (void *)&makeclosurekey);
    luaL_dostring(L, "return function (f, ...)\n"
                  "local args = {...}\n"
                  "return (function () return f(" LUA_UNPACK "(args)) end)\n"
                  "end\n");
    lua_settable(L, LUA_REGISTRYINDEX);

    return 0;
  }
}

Lua::Lua() {
  L_ = luaL_newstate();
  if (L_) {
    lua_pushlightuserdata(L_, (void *)&LuaImpl::luakey);
    lua_pushlightuserdata(L_, (void *)this);
    lua_settable(L_, LUA_REGISTRYINDEX);

    lua_pushcfunction(L_, &LuaImpl::pmain);
    int status = lua_pcall(L_, 0, 1, 0);
    if (status != LUA_OK) {
      const char *e = lua_tostring(L_, -1);
      printf("lua init(err=%d): %s\n", status, e);
    }
  }
}

Lua::~Lua() {
  printf("lua_exit\n");
  lua_close(L_);
}

Lua *Lua::from_state(lua_State *L) {
  Lua *lua;
  lua_pushlightuserdata(L, (void *)&LuaImpl::luakey);
  lua_gettable(L, LUA_REGISTRYINDEX);
  lua = (Lua *) lua_touserdata(L, -1);
  lua_pop(L, 1);
  return lua;
}

an<LuaObj> Lua::newthread(lua_State *L, int nargs) {
  lua_State *C = lua_newthread(L_);
  auto o = LuaObj::todata(L_, -1);
  lua_pop(L_, 1);

  lua_pushlightuserdata(C, (void *)&LuaImpl::makeclosurekey);
  lua_gettable(C, LUA_REGISTRYINDEX);
  lua_xmove(L, C, 1 + nargs);
  lua_call(C, 1 + nargs, 1);

  return o;
}

LuaObj::LuaObj(lua_State *L, int i) : L_(L) {
  lua_pushvalue(L, i);
  id_ = luaL_ref(L, LUA_REGISTRYINDEX);
}

LuaObj::~LuaObj() {
  luaL_unref(L_, LUA_REGISTRYINDEX, id_);
}

void LuaObj::pushdata(lua_State *L, an<LuaObj> &o) {
  lua_rawgeti(L, LUA_REGISTRYINDEX, o->id_);
}

an<LuaObj> LuaObj::todata(lua_State *L, int i) {
  return an<LuaObj>(new LuaObj(L, i));
}

}  // namespace rime
