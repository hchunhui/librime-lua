/*
 * script_translator.h
 * Copyright (C) 2023 Shewer Lu <shewer@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef _LUA_TRANSLATOR_H
#define _LUA_TRANSLATOR_H

#include <rime/common.h>
#include <rime/engine.h>
#include <rime/schema.h>
#include <rime/config.h>
#include "lib/lua_export_type.h"

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
  rime::an<O> obj = rime::New<O>(ticket, lua);
  if (obj) {
    LuaType<rime::an<O>>::pushdata(L, obj);
    return 1;
  }
  else {
    //LOG(ERROR) << "error creating " << typeid(O).name() << ": '" << ticket.klass << "'";
    return 0;
  }
};
#endif /* !_LUA_TRANSLATOR_H */
