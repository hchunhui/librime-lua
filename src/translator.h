/*
 * script_translator.h
 * Copyright (C) 2023 Shewer Lu <shewer@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef _LUA_TRANSLATOR_H
#define _LUA_TRANSLATOR_H

#include <rime/ticket.h>
#include "lib/lua_export_type.h"
#include "optional.h"

#define SET_(name, type)             \
  void set_##name(const type data) { \
    name##_ = data;                  \
  }
#define GET_(name, type) \
  type name() {          \
    return name##_;      \
  }
#define ACCESS_(name, type) \
  SET_(name, type);         \
  GET_(name, type)

#define Set_WMEM(name) \
  { #name, WRAPMEM(T, set_##name) }
#define WMEM(name) \
  { #name, WRAPMEM(T, name) }
#define Get_WMEM(name) \
  { #name, WRAPMEM(T, name) }

#if LUA_VERSION_NUM < 502
#define lua_absindex(L, i) abs_index((L), (i))
#endif


template <typename O>
int raw_make_translator(lua_State* L){
  int n = lua_gettop(L);
  if (3 > n || 4 < n)
    return 0;

  C_State C;
  rime::Ticket ticket(
      LuaType<rime::Engine *>::todata(L, 1),
      LuaType<std::string>::todata(L, -2, &C),
      LuaType<std::string>::todata(L, -1, &C)
      );
  DLOG(INFO) << "check Ticket:" << ticket.klass << "@" <<ticket.name_space ;
  if ( n == 4 )
    ticket.schema = &(LuaType<rime::Schema &>::todata(L, 2) ); //overwrite schema
  Lua* lua= Lua::from_state(L);
  std::shared_ptr<O> obj = rime::New<O>(ticket, lua);
  if (obj) {
    LuaType<std::shared_ptr<O>>::pushdata(L, obj);
    return 1;
  }
  else {
    //LOG(ERROR) << "error creating " << typeid(O).name() << ": '" << ticket.klass << "'";
    return 0;
  }
};

template <typename O>
int raw_set_memorize_callback(lua_State *L) {
  bool res = false;
  auto t = LuaType<std::shared_ptr<O>>::todata(L, 1);
  const int type = (1 < lua_gettop(L)) ? lua_type(L, 2) : LUA_TNIL;
  if (type == LUA_TNIL) {
    // reset memorize_callback
    LOG(INFO) << typeid(*t).name() <<" of " << t->name_space() << ": reset memorize_callback";
    t->set_memorize_callback({});
    res = true;
  }
  else if (type == LUA_TFUNCTION) {
    t->set_memorize_callback( LuaObj::todata(L, 2));
    res = true;
  }
  else {
    LOG(WARNING) << typeid(*t).name() <<" of " << t->name_space()
		 << ": set memorize_callback '?' (function expected, got " << lua_typename(L, type) <<")";
  }
  lua_pushboolean(L, res);
  return 1;
}
#endif /* !_LUA_TRANSLATOR_H */
