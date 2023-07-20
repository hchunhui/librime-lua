#include <cstdio>
#include <rime/common.h>
#include <rime/registry.h>
#include <rime_api.h>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include "lib/lua_templates.h"
#include "lua_gears.h"

namespace fs = boost::filesystem;

void types_init(lua_State *L);

static void lua_init(lua_State *L) {
  const auto user_dir = std::string(RimeGetUserDataDir());
  const auto shared_dir = std::string(RimeGetSharedDataDir());

  types_init(L);
  lua_getglobal(L, "package");
  lua_pushfstring(L, "%s%slua%s?.lua;"
                  "%s%slua%s?%sinit.lua;"
                  "%s%slua%s?.lua;"
                  "%s%slua%s?%sinit.lua;",
                  user_dir.c_str(), LUA_DIRSEP, LUA_DIRSEP,
                  user_dir.c_str(), LUA_DIRSEP, LUA_DIRSEP, LUA_DIRSEP,
                  shared_dir.c_str(), LUA_DIRSEP, LUA_DIRSEP,
                  shared_dir.c_str(), LUA_DIRSEP, LUA_DIRSEP, LUA_DIRSEP);
  lua_getfield(L, -2, "path");
  lua_concat(L, 2);
  lua_setfield(L, -2, "path");
  lua_pop(L, 1);

  boost::regex pattern("rime(\\..*)*\\.lua");
  bool find_file_in_user_dir = false;
  // search in user_dir
  for(const auto& entry : fs::directory_iterator(user_dir)){
	if(boost::regex_match(entry.path().filename().string(), pattern)){
		find_file_in_user_dir = true;
		if(luaL_dofile(L, entry.path().string().c_str())){
			const char *e = lua_tostring(L, -1);
			LOG(ERROR) << entry.path().string() << " error: " << e;
			lua_pop(L, 1);
		}
	}
  }
  // search in shared_dir
  if(!find_file_in_user_dir){
	  for(const auto& entry : fs::directory_iterator(shared_dir)){
		  if(boost::regex_match(entry.path().filename().string(), pattern)){
			  find_file_in_user_dir = true;
			  if (luaL_dofile(L, entry.path().string().c_str())) {
				  const char *e = lua_tostring(L, -1);
				  LOG(ERROR) << entry.path().string() << " error: " << e;
				  lua_pop(L, 1);
			  }
		  }
	  }
  }
  // error if "rime(\\..*)*\\.lua" not found in either user_dir and shared_dir
  if(!find_file_in_user_dir){
    LOG(INFO) << "rime.lua info: rime.lua should be either in the "
                 "rime user data directory or in the rime shared "
                 "data directory";
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
