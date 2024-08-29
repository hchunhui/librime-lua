#ifndef LUA_COMPAT_H
#define LUA_COMPAT_H

#if LUA_VERSION_NUM == 501 && !defined(LUA_OK)
#define LUA_OK 0
#define luaL_newlib(L, l) (lua_newtable((L)),luaL_setfuncs((L), (l), 0))

void luaL_setmetatable (lua_State *L, const char *tname);
void luaL_setfuncs (lua_State *L, const luaL_Reg *l, int nup);
#endif

#if LUA_VERSION_NUM >= 504
static inline int xlua_resume(lua_State *L, int nargs)
{
	int nres;
	return lua_resume(L, NULL, nargs, &nres);
}
#elif LUA_VERSION_NUM >= 502
#define xlua_resume(L, a) lua_resume(L, NULL, a)
#else
#define xlua_resume lua_resume
#define lua_rawlen lua_objlen
#endif

void xluaopen_utf8(lua_State *);

#if LUA_VERSION_NUM < 504
LUALIB_API int luaL_typeerror(lua_State *L, int arg, const char *tname);
#endif

#endif /* LUA_COMPAT_H */
