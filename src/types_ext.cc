/*
 * types_ext.cc
 * Copyright (C) 2022 Shewer Lu <shewer@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#include <rime/common.h>
#include <rime/processor.h>
#include <rime/segmentor.h>
#include <rime/translator.h>
#include <rime/filter.h>
#include <rime/dict/reverse_lookup_dictionary.h>
#include <rime/dict/user_db.h>

#include "lib/lua_export_type.h"
#include "lib/luatype_boost_optional.h"

#include <utility>

using namespace rime;

namespace {

template<typename> using void_t = void;

template<typename T, typename = void>
struct COMPAT {
  // fallback version of name_space() if librime is old
  static nullptr_t name_space(T &t) {
    return nullptr;
  }
};

template<typename T>
struct COMPAT<T, void_t<decltype(std::declval<T>().name_space())>> {
  static std::string name_space(T &t) {
    return t.name_space();
  }
};

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
    {"name_space",WRAP(COMPAT<T>::name_space)},
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
    {"name_space",WRAP(COMPAT<T>::name_space)},
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
    {"name_space",WRAP(COMPAT<T>::name_space)},
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
    {"name_space",WRAP(COMPAT<T>::name_space)},
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
namespace UserDbReg{
  typedef Db T;
  typedef DbAccessor A;

  an<T> make(const string& db_name, const string& db_class) {
    if (auto comp= UserDb::Require(db_class)){
      return an<T>( comp->Create(db_name)) ;
    }
    else {
      return {};
    }
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

  bool Open(T &t) { return t.Open(); }
  bool Close(T &t) { return t.Close(); }
  bool OpenReadOnly(T &t) { return t.OpenReadOnly(); }
  bool Erase(T &t, const string &key) { return t.Erase(key); }
  bool Update(T &t, const string &key, const string &value) {
    return t.Update(key, value);
  }

  an<A> Query(T &t, const string& key) { return t.Query(key); }
  
  static const luaL_Reg funcs[] = {
    {"UserDb", WRAP(make)},// an<Db> LevelDb( db_file, db_name)
    {"LevelDb", WRAP(make_leveldb)},// an<Db> LevelDb( db_file, db_name)
    {"TableDb", WRAP(make_tabledb)},// Db UserDb( db_name, db_type:userdb|plain_userdb)
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    {"open", WRAP(Open)},
    {"open_read_only", WRAP(OpenReadOnly)},
    {"close", WRAP(Close)},
    {"query", WRAP(Query)}, // query(prefix_key) return DbAccessor
    {"fetch", WRAP(fetch)},  //   fetch(key) return value
    {"update", WRAP(Update)}, // update(key,value) return bool
    {"erase", WRAP(Erase)}, // erase(key) return bool

    {"disable", WRAPMEM(T, disable)},
    {"enable", WRAPMEM(T, enable)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    {"loaded",WRAPMEM(T, loaded)},
    {"read_only",WRAPMEM(T, readonly)},
    {"disabled",WRAPMEM(T, disabled)},
    {"name", WRAPMEM(T, name)},
    {"file_name", WRAPMEM(T, file_name)},
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

}

void LUAWRAPPER_LOCAL types_ext_init(lua_State *L) {
  EXPORT(ProcessorReg, L);
  EXPORT(SegmentorReg, L);
  EXPORT(TranslatorReg, L);
  EXPORT(FilterReg, L);
  EXPORT(ReverseLookupDictionaryReg, L);
  EXPORT(DbAccessorReg, L);
  EXPORT(UserDbReg, L);
  ComponentReg::init(L);
}
