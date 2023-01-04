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
#include <rime/dict/dictionary.h>
#include <rime/dict/user_dictionary.h>
#include <rime/dict/level_db.h>

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

namespace TicketReg {
  typedef  Ticket T;

  int raw_make(lua_State* L) {
    T t;
    int n = lua_gettop(L);

    if ( 2 == n ) {
      // Ticket( *schema, ns )
      t =Ticket(
            &LuaType<Schema &>::todata(L,1),
            lua_tostring(L,2));
    }
    else if ( 3 == n ) {
      // Ticket( *eng , ns , prescription)
      t= Ticket(
            LuaType<Engine *>::todata(L,1),
            lua_tostring(L,2),
            lua_tostring(L,3) );
    }
    else if ( n == 0) { } // return E
    else {
      return 0;
    }
    LuaType<T>::pushdata(L,t);
    return 1;
  }
  void set_schema(T &t, Schema &s){
    t.schema= &s;
  }


  static const luaL_Reg funcs[] = {
    { "Ticket", raw_make},
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    {"engine", WRAPMEM_GET(T::engine)},
    {"schema", WRAPMEM_GET(T::schema)},
    {"name_space", WRAPMEM_GET(T::name_space)},
    {"klass", WRAPMEM_GET(T::klass)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    {"engine", WRAPMEM_SET(T::engine)},
    {"schema", WRAP(set_schema)},
    {"name_space", WRAPMEM_SET(T::name_space)},
    {"klass", WRAPMEM_SET(T::klass)},
    { NULL, NULL },
  };


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
// Dictionary
namespace DictionaryReg {
  typedef Dictionary T;
  typedef DictionaryComponent C;

  int raw_make(lua_State* L) {
    int n = lua_gettop(L);
    if (n == 0 )
      return 0;

    auto c = (C *) T::Require("dictionary");
    if (!c)
      return 0;

    T * dict=nullptr;
    if (lua_isstring(L, 1)){
      // from  dict_name , pirsm , packs
      vector<string> packs;
      string dictname, prism;
      if ( 1 == n ){
        dictname = string( lua_tostring(L, 1) );
        prism = dictname;
      }
      else if (2 == n) {
        dictname = string( lua_tostring(L, 1) );
        prism = string( lua_tostring(L, 2) );
      }
      else if (3 <= n) {
        dictname = string( lua_tostring(L, 1) );
        prism = string( lua_tostring(L, 2) );
        for (int i=3; i<n; i++)
          packs.push_back(lua_tostring(L, i) );
      }
      dict = c->Create(dictname, prism, packs);
    }
    else {
      // from name_space
      Ticket t = LuaType<Ticket &>::todata(L, 1);
      dict = c->Create(t);
    }

    if ( dict ) {
      LuaType<T *>::pushdata(L, dict);
      return 1;
    };
    return 0;
  }

  static const luaL_Reg funcs[] = {
    {"Dictionary",raw_make},
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };
}

namespace UserDictionaryReg {
  typedef UserDictionary T;
  typedef UserDictionaryComponent C;

  int raw_make(lua_State* L) {
    int n = lua_gettop(L);
    if (n == 0 )
      return 0;

    auto c = (C *) T::Require("user_dictionary");
    if (!c)
      return 0;

    T * dict=nullptr;
    if (lua_isstring(L, 1)){
      string user_dictname, dbclass;
      if ( 1 == n ){
        user_dictname= string( lua_tostring(L, 1) );
        dbclass= user_dictname;
      }
      else if (2 == n) {
        user_dictname= string( lua_tostring(L, 1) );
        dbclass= string( lua_tostring(L, 2) );
      }
      else if (3 == n) {
        user_dictname= string( lua_tostring(L, 1) );
        dbclass= string( lua_tostring(L, 2) );
      }
      dict = c->Create( user_dictname, dbclass);
    }
    else {
      Ticket t = LuaType<Ticket &>::todata(L, 1);
      dict = c->Create(t);
    }

    if ( dict ) {
      LuaType<T *>::pushdata(L, dict);
      return 1;
    };
    return 0;
  }

  static const luaL_Reg funcs[] = {
    {"UserDictionary", raw_make},
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
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
    // args : (Ticket | engine, ns,pres | engine, schema, ns, pres)
    C_State C;
    Ticket ticket;

    if (n == 1){
      ticket = LuaType<Ticket &>::todata(L,1);
    }
    else if ( 3 <= n ){
      ticket = Ticket(
        LuaType<Engine *>::todata(L, 1),
        LuaType<string>::todata(L, -2, &C),
        LuaType<string>::todata(L, -1, &C));
      if (n == 4)
        ticket.schema = &(LuaType<Schema &>::todata(L, 2) ); //overwrite schema
    }
    else
      return 0;

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
  EXPORT(TicketReg, L);
  EXPORT(ProcessorReg, L);
  EXPORT(SegmentorReg, L);
  EXPORT(TranslatorReg, L);
  EXPORT(FilterReg, L);
  EXPORT(DictionaryReg, L);
  EXPORT(UserDictionaryReg, L);
  EXPORT(ReverseLookupDictionaryReg, L);
  EXPORT(DbAccessorReg, L);
  EXPORT(LevelDbReg, L);
  ComponentReg::init(L);
}
