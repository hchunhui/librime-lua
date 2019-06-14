#ifndef LUA_COMPAT_H
#define LUA_COMPAT_H

#if LUA_VERSION_NUM == 501 && !defined(LUA_OK)
#define LUA_OK 0
#define luaL_newlib(L, l) (lua_newtable((L)),luaL_setfuncs((L), (l), 0))

void luaL_setmetatable (lua_State *L, const char *tname);
void luaL_setfuncs (lua_State *L, const luaL_Reg *l, int nup);
#endif

#if LUA_VERSION_NUM >= 502
#define xlua_resume(L, a) lua_resume(L, NULL, a)
#define LUA_UNPACK "table.unpack"
#else
#define xlua_resume lua_resume
#define lua_rawlen lua_objlen
#define LUA_UNPACK "unpack"
#endif

void xluaopen_utf8(lua_State *);

#endif /* LUA_COMPAT_H */
