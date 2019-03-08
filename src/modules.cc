#include <rime/common.h>
#include <rime/registry.h>
#include <rime_api.h>
#include "lib/lua_templates.h"
#include "lua_gears.h"

void types_init(lua_State *L);

static void lua_init(lua_State *L) {
  auto file = std::string(RimeGetUserDataDir()) + "/rime.lua";
  types_init(L);
  int status = luaL_dofile(L, file.c_str());
  if (status != LUA_OK) {
    const char *e = lua_tostring(L, -1);
    LOG(INFO) << "lua_init error(" << status << "): " << e;
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
