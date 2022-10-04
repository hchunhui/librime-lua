#ifndef LUA_EXPORT_TYPE_H
#define LUA_EXPORT_TYPE_H
#include "lua_templates.h"

void lua_export_type(lua_State *L,
                     const LuaTypeInfo *type, lua_CFunction gc,
                     const luaL_Reg *funcs, const luaL_Reg *methods,
                     const luaL_Reg *vars_get, const luaL_Reg *vars_set);

#define EXPORT_TYPE(ns, L) \
  lua_export_type(L, LuaType<ns::T>::type(), LuaType<ns::T>::gc,       \
                  ns::funcs, ns::methods, ns::vars_get, ns::vars_set)

#define EXPORT_CST_TYPE(ns, L) \
  lua_export_type(L, LuaType<const ns::T>::type(), LuaType<ns::T>::gc, \
                  ns::funcs, ns::methods, ns::vars_get, ns::vars_set)

#define EXPORT_REF_TYPE(ns, L) \
  lua_export_type(L, LuaType<ns::T &>::type(), NULL,                   \
                  ns::funcs, ns::methods, ns::vars_get, ns::vars_set)

#define EXPORT_REF_CST_TYPE(ns, L) \
  lua_export_type(L, LuaType<const ns::T &>::type(), NULL,             \
                  ns::funcs, ns::methods, ns::vars_get, ns::vars_set)

#define EXPORT_PTR_TYPE(ns, L) \
  lua_export_type(L, LuaType<ns::T *>::type(), NULL,                   \
                  ns::funcs, ns::methods, ns::vars_get, ns::vars_set)

#define EXPORT_PTR_CST_TYPE(ns, L) \
  lua_export_type(L, LuaType<const ns::T *>::type(), NULL,             \
                  ns::funcs, ns::methods, ns::vars_get, ns::vars_set)

#define EXPORT_SPTR_TYPE(ns, L) \
  lua_export_type(L, LuaType<std::shared_ptr<ns::T>>::type(),          \
                  LuaType<std::shared_ptr<ns::T>>::gc,                 \
                  ns::funcs, ns::methods, ns::vars_get, ns::vars_set)

#define EXPORT_SPTR_CST_TYPE(ns, L) \
  lua_export_type(L, LuaType<std::shared_ptr<const ns::T>>::type(),    \
                  LuaType<std::shared_ptr<const ns::T>>::gc,           \
                  ns::funcs, ns::methods, ns::vars_get, ns::vars_set)

#define EXPORT_UPTR_TYPE(ns, L) \
  lua_export_type(L, LuaType<std::unique_ptr<ns::T>>::type(),          \
                  LuaType<std::unique_ptr<ns::T>>::gc,                 \
                  ns::funcs, ns::methods, ns::vars_get, ns::vars_set)

#define EXPORT(ns, L) \
  do {                         \
  EXPORT_TYPE(ns, L);          \
  EXPORT_REF_TYPE(ns, L);      \
  EXPORT_CST_TYPE(ns, L);      \
  EXPORT_REF_CST_TYPE(ns, L);  \
  EXPORT_SPTR_TYPE(ns, L);     \
  EXPORT_SPTR_CST_TYPE(ns, L); \
  EXPORT_PTR_TYPE(ns, L);      \
  EXPORT_PTR_CST_TYPE(ns, L);  \
  } while (0)

#endif /* LUA_EXPORT_TYPE_H */
