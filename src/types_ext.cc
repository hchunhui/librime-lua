/*
 * types_ext.cc
 * Copyright (C) 2022 Shewer Lu <shewer@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#include <cstddef>
#include <rime/common.h>
#include <rime/processor.h>
#include <rime/segmentor.h>
#include <rime/translator.h>
#include <rime/filter.h>
#include <rime/dict/reverse_lookup_dictionary.h>
#include <rime/dict/user_db.h>

#include "lib/lua_export_type.h"
#include "optional.h"
#include <utility>

using namespace rime;
namespace {

template<typename> using void_t = void;

template<typename T, typename = void>
struct COMPAT {
  // fallback version of name_space() if librime is old
  static std::nullptr_t name_space(T &t) {
    return nullptr;
  }
};

template<typename T>
struct COMPAT<T, void_t<decltype(std::declval<T>().name_space())>> {
  static std::string name_space(T &t) {
    return t.name_space();
  }
};

// fallback version of file_path() if librime is old
template<typename> struct void_t1 { using t = int; };
template<typename T, typename void_t1<decltype(std::declval<T>().file_name())>::t = 0>
std::string get_UserDb_file_path_string(const T &t) {
    return t.file_name();
}

template<typename T, typename void_t1<decltype(std::declval<T>().file_path())>::t = 0>
std::string get_UserDb_file_path_string(const T &t) {
    return t.file_path().string();
}

namespace ProcessorReg{
  using T = Processor;

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
    {"name_space",WRAP(COMPAT<T>::name_space)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };

}

namespace SegmentorReg{
  using T = Segmentor;

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
    {"name_space",WRAP(COMPAT<T>::name_space)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };
}

namespace TranslatorReg{
  using T = Translator;

  static const luaL_Reg funcs[] = {
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    {"query",WRAPMEM(T::Query)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    {"name_space",WRAP(COMPAT<T>::name_space)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };
}

namespace FilterReg{
  using T = Filter;

  static const luaL_Reg funcs[] = {
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    {"apply",WRAPMEM(T::Apply)},
    {"applies_to_segment",WRAPMEM(T::AppliesToSegment)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    {"name_space",WRAP(COMPAT<T>::name_space)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };
}
// ReverseDictionary
namespace ReverseLookupDictionaryReg {
  using T = ReverseLookupDictionary;
  using C = ReverseLookupDictionaryComponent;

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
  using T = DbAccessor;

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
namespace UserDbReg{
  using T = Db;
  using A = DbAccessor;

  an<T> make(const string& db_name, const string& db_class) {
    if (auto comp= Db::Require(db_class)) {
      return an<T>(comp->Create(db_name));
    }
    return {};
  }

  an<T> make_leveldb(const string& db_name) {
    return make(db_name, "userdb");
  }

  an<T> make_tabledb(const string& db_name) {
    return make(db_name, "plain_userdb");
  }

  optional<string> fetch(an<T> t, const string& key) {
    string res;
    if ( t->Fetch(key,&res) )
      return res;
    return {};
  }

  static const luaL_Reg funcs[] = {
    {"UserDb", WRAP(make)},// an<Db> LevelDb( db_file, db_name)
    {"LevelDb", WRAP(make_leveldb)},// an<Db> LevelDb( db_file, db_name)
    {"TableDb", WRAP(make_tabledb)},// Db UserDb( db_name, db_type:userdb|plain_userdb)
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    {"open", WRAPMEM(T, Open)},
    {"open_read_only", WRAPMEM(T, OpenReadOnly)},
    {"close", WRAPMEM(T, Close)},
    {"query", WRAPMEM(T, Query)}, // query(prefix_key) return DbAccessor
    {"fetch", WRAP(fetch)},  //   fetch(key) return value
    {"update", WRAPMEM(T, Update)}, // update(key,value) return bool
    {"erase", WRAPMEM(T, Erase)}, // erase(key) return bool

    {"loaded",WRAPMEM(T, loaded)},
    {"disable", WRAPMEM(T, disable)},
    {"enable", WRAPMEM(T, enable)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    {"_loaded",WRAPMEM(T, loaded)},
    {"read_only",WRAPMEM(T, readonly)},
    {"disabled",WRAPMEM(T, disabled)},
    {"name", WRAPMEM(T, name)},
    {"file_name", WRAP(get_UserDb_file_path_string<T>)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };
}

namespace ComponentReg{
  using P = Processor;
  using S = Segmentor;
  using T = Translator;
  using F = Filter;

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

}

void table_translator_init(lua_State *L);
void script_translator_init(lua_State *L);

void LUAWRAPPER_LOCAL types_ext_init(lua_State *L) {
  EXPORT(ProcessorReg, L);
  EXPORT(SegmentorReg, L);
  EXPORT(TranslatorReg, L);
  EXPORT(FilterReg, L);
  EXPORT(ReverseLookupDictionaryReg, L);
  EXPORT(DbAccessorReg, L);
  EXPORT(UserDbReg, L);
  ComponentReg::init(L);
  // add LtableTranslator ScriptTranslator in Component
  table_translator_init(L);
  script_translator_init(L);
}
