#include <rime/common.h>
#include <rime/registry.h>
#include <rime_api.h>
#include "lib/lua_templates.h"
#include "lua_gears.h"

void types_init(lua_State *L);

static void lua_init(lua_State *L) {
  const char *user_dir = RimeGetUserDataDir();
  types_init(L);
  lua_getglobal(L, "package");
  lua_pushfstring(L, "%s%slua%s?.lua;%s%slua%s?%sinit.lua;",
                  user_dir, LUA_DIRSEP, LUA_DIRSEP,
                  user_dir, LUA_DIRSEP, LUA_DIRSEP, LUA_DIRSEP);
  lua_getfield(L, -2, "path");
  lua_concat(L, 2);
  lua_setfield(L, -2, "path");
  lua_pop(L, 1);

  auto file = std::string(user_dir) + "/rime.lua";
  if (luaL_dofile(L, file.c_str())) {
    const char *e = lua_tostring(L, -1);
    LOG(INFO) << "rime.lua error: " << e;
    lua_pop(L, 1);
  }
}

static void rime_lua_initialize() {
  using namespace rime;

  LOG(INFO) << "registering components from module 'lua'.";
  Registry& r = Registry::instance();

  an<Lua> lua(new Lua);
  lua->to_state(lua_init);

  r.Register("lua_translator", new LuaComponent<LuaTranslator>(lua));
  r.Register("lua_filter", new LuaComponent<LuaFilter>(lua));
  r.Register("lua_segmentor", new LuaComponent<LuaSegmentor>(lua));
  r.Register("lua_processor", new LuaComponent<LuaProcessor>(lua));
}

static void rime_lua_finalize() {
}

RIME_REGISTER_MODULE(lua)
