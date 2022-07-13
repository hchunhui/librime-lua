/*
 * types_ext.cc
 * Copyright (C) 2022 Shewer Lu <shewer@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef _TYPE_EXT_INC
#define _TYPE_EXT_INC
#include <rime/common.h>
#include <rime/dict/level_db.h>
using namespace rime;

namespace ProcessorReg{
  typedef Processor T;

  int process_key_event(T &t, const KeyEvent &key){
    switch (t.ProcessKeyEvent(key) ){
      case kRejected: return kRejected;
      case kAccepted: return kAccepted;
      default: return kNoop;
    }
  }

  static const luaL_Reg funcs[] = {
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    {"process_key_event",WRAP(process_key_event)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };

}

namespace SegmentorReg{
  typedef Segmentor T;

  bool proceed(T &t, Segmentation & s) {
    return t.Proceed(&s);
  }

  static const luaL_Reg funcs[] = {
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    {"proceed",WRAP(proceed)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };
}

namespace TranslatorReg{
  typedef Translator T;

  static const luaL_Reg funcs[] = {
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    {"query",WRAPMEM(T::Query)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };
}

namespace FilterReg{
  typedef Filter T;

  static const luaL_Reg funcs[] = {
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    {"apply",WRAPMEM(T::Apply)},
    {"applies_to_segment",WRAPMEM(T::AppliesToSegment)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };
}
// ReverseDictionary
namespace ReverseLookupDictionaryReg {
  typedef ReverseLookupDictionary T;
  typedef ReverseLookupDictionaryComponent C;

  an<T> make(const string& dict_name) {
    if ( auto c = (C *) T::Require("reverse_lookup_dictionary")){
      auto t = (an<T>) c->Create(dict_name);
      if ( t  && t->Load())
        return t;
    };
    return {};
  }

  string lookup(T& db, const string &key){
    string res;
    return ( db.ReverseLookup(key, &res) ) ? res : string("") ;
  }

  string lookup_stems(T& db, const string &key){
    string res;
    return ( db.LookupStems(key, &res) ) ? res : string("") ;
  }

  static const luaL_Reg funcs[] = {
    {"ReverseLookup",WRAP(make)},
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    {"lookup",WRAP(lookup)},
    {"lookup_stems",WRAP(lookup_stems)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };
}

// leveldb
namespace DbAccessorReg{
  typedef DbAccessor T;

  // return key , value or nil
  int raw_next(lua_State* L){
    int n = lua_gettop(L);
    if (1 > n )
      return 0;
    auto a = LuaType<an<T>>::todata(L, 1);
    string key,value;
    if (a->GetNextRecord(&key,&value)) {
      LuaType<string>::pushdata(L,key);
      LuaType<string>::pushdata(L,value);
      return 2;
    }else {
      return 0;
    }
  };

  // return raw_next, self
  // for key,value in peek, self do print(key,value) end
  int raw_iter(lua_State* L){
    int n = lua_gettop(L);
    if (1 > n)
      return 0;
    lua_pushcfunction(L, raw_next);
    lua_insert(L, 1);
    //lua_rotate(L, 2, 1);
    lua_settop(L, 2);
    return 2;
  }

  static const luaL_Reg funcs[] = {
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    {"reset", WRAPMEM(T::Reset)},
    {"jump", WRAPMEM(T::Jump)},
    {"iter",raw_iter},
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };
}
namespace LevelDbReg{
  typedef LevelDb T;
  typedef DbAccessor A;

  //
  an<T> make(const string& file_name, const string& db_name){
    return New<LevelDb>(file_name,db_name,"userdb");
  }
  optional<string> fetch(an<T> t, const string& key){
    string res;
    if ( t->Fetch(key,&res) )
      return res;
    return {};
  }

  bool loaded(an<T> t){
    return t->loaded();
  }

  static const luaL_Reg funcs[] = {
    {"LevelDb", WRAP(make)},
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    {"open", WRAPMEM(T::Open)},
    {"open_read_only", WRAPMEM(T::OpenReadOnly)},
    {"close", WRAPMEM(T::Close)},
    {"query", WRAPMEM(T::Query)}, // query(prefix_key) return DbAccessor
    {"fetch", WRAP(fetch)},  //   fetch(key) return value
    {"update", WRAPMEM(T::Update)}, // update(key,value) return bool
    {"erase", WRAPMEM(T::Erase)}, // erase(key) return bool
    {"loaded",WRAPMEM(T,loaded)},

    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };
}

namespace ComponentReg{
  typedef Processor P;
  typedef Segmentor S;
  typedef Translator T;
  typedef Filter F;

  template <typename O>
  int raw_create(lua_State *L){
    int n = lua_gettop(L);
    if (3 > n || 4 < n)
      return 0;

    C_State C;
    Ticket ticket(
      LuaType<Engine *>::todata(L, 1),
      LuaType<string>::todata(L, -2, &C),
      LuaType<string>::todata(L, -1, &C)
    );
    if (n == 4)
      ticket.schema = &(LuaType<Schema &>::todata(L, 2) ); //overwrite schema

    if (auto c = O::Require(ticket.klass)) {
      an<O> obj = (an<O>) c->Create(ticket);
      LuaType<an<O>>::pushdata(L, obj);
      return 1;
    }
    else {
      LOG(ERROR) << "error creating " << typeid(O).name() << ": '" << ticket.klass << "'";
      return 0;
    }
  };

  static const luaL_Reg funcs[] = {
    {"Processor",  raw_create<P>},
    {"Segmentor"   , raw_create<S>},
    {"Translator", raw_create<T>},
    {"Filter", raw_create<F>},
    { NULL, NULL },
  };

  void init(lua_State *L) {
    lua_createtable(L, 0, 0);
    luaL_setfuncs(L, funcs, 0);
    lua_setglobal(L, "Component");
  }
}

#define EXPORT_TYPES_EXT(L) \
{\
  EXPORT(ProcessorReg, L);\
  EXPORT(SegmentorReg, L);\
  EXPORT(TranslatorReg, L);\
  EXPORT(FilterReg, L);\
  EXPORT(ReverseLookupDictionaryReg, L);\
  EXPORT(DbAccessorReg, L);\
  EXPORT(LevelDbReg, L);\
  ComponentReg::init(L);\
}

#endif /* ifndef _TYPE_EXT_INC */


