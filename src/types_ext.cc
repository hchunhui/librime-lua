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
#include "table_translator.h"

#include <rime/gear/poet.h>
#include <rime/gear/unity_table_encoder.h>

#include "lib/lua_export_type.h"
#include "lib/luatype_boost_optional.h"

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
    {"file_name", WRAPMEM(T, file_name)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };
}
#define Set_WMEM(name) {#name, WRAPMEM(T, set_##name)}
#define WMEM(name) {#name, WRAPMEM(T, name)}
#define Get_WMEM(name) {#name, WRAPMEM(T, name)}

#if LUA_VERSION_NUM < 502
#default lua_absindex(L, i) abs_index((L), (i))
#endif

namespace TableTranslatorReg {
  using T = LTableTranslator;

  static const luaL_Reg funcs[] = {
    { NULL, NULL },
  };
  int raw_get_table(lua_State* L) {
    if (lua_gettop(L) < 1)
      return 0;

    if (!luaL_getmetafield(L, 1,"vars_get"))// obj, ...  mt['vars_get']
      return 0;
    if (lua_type(L, -1) != LUA_TTABLE)
      return 0;

    lua_newtable(L); // 
    lua_pushnil(L);//obj,...mt['vars_get'], restab, key(nil)
    while (lua_next(L, -2)) { //next , va_index, key
      if (lua_type(L, -2) == LUA_TSTRING && lua_type(L, -1) == LUA_TFUNCTION) {
        lua_pushvalue(L, 1);// restab, key , func , obj
        if (lua_pcall(L, 1, 1, 0)) { // restab, key, value  
          lua_setfield(L, -3, lua_tostring(L, -2));// restab key
          continue;
        }
      }
      lua_pop(L, 1); //remove dont carevalue
    }// end of while : obj, .. mt[vars_get'] , res_table
    return 1;
  }// raw_get_table

  int raw_set_table(lua_State* L) {
    if (lua_gettop(L) < 2)
      return 0;
    if (lua_type(L, 2) != LUA_TTABLE)
      return 0;
    if (!luaL_getmetafield(L, 1,"vars_set"))// +mt['vars_set']
      return 0;

    int vs_index = lua_absindex(L, -1);
    lua_pushnil(L); // obj, set_tab,... mt['vars_set'], key(nil)
    while(lua_next(L, 2)) {// next ,set_tab...key
      if (lua_type(L, -2) == LUA_TSTRING) {// mt['vars_set'],skey, svalue
        //char* key= lua_tostring(L, -2);
        lua_getfield(L, vs_index, lua_tostring(L, -2)); 
        if (lua_type(L, -1) == LUA_TFUNCTION) { // mt['vars_set'] skey, svalue  vset_func
           lua_pushvalue(L, 1);
           lua_pushvalue(L, -3);// skey, svalue, vset_func, obj, svalue
           if (lua_pcall(L, 3, 1, 0) != LUA_OK) { // mt['vars_set'], skey svalue, donet_care
             LOG(WARNING) << lua_tostring(L, -1);
           }
        }
        lua_pop(L, 1);//skey svalue remove ( vsvalue or donet_care) 
      }
      lua_pop(L, 1); // skey remove (svalue)
    }//end of while
    return 0;
  }//raw_set_table

  int raw_set_table1(lua_State* L) {
    if (lua_gettop(L) >= 2 && lua_type(L, 2) == LUA_TTABLE &&
        luaL_getmetafield(L, 1,"vars_set")){// +mt['vars_set']

      int vs_index = lua_absindex(L, -1);
      lua_pushnil(L); // obj, set_tab,... mt['vars_set'], key(nil)
      while (lua_next(L, 2)) {// next ,set_tab...key
        if (lua_type(L, -2) == LUA_TSTRING) { // mt['vars_set'],skey, svalue
          //char* key= lua_tostring(L, -2);
          lua_getfield(L, vs_index, lua_tostring(L, -2));
          if (lua_type(L, -1) != LUA_TFUNCTION) { // mt['vars_set'] skey, svalue  vset_func
            lua_pop(L, 2); 
            continue;
          }
          lua_pushvalue(L, 1);
          lua_pushvalue(L, -3);// skey, svalue, vset_func, obj, svalue
                               // 
          if (lua_pcall(L, 3, 1, 0) != LUA_OK) { // mt['vars_set'], skey svalue, donet_care
            LOG(WARNING) << lua_tostring(L, -1);
          }
        }
        lua_pop(L, 1); // skey remove (svalue)
      }//end of while
    }// end of if
    return 0;
  }//raw_set_table
  static const luaL_Reg methods[] = {
    {"query", WRAPMEM(T, Query)},// string, segment
    WMEM(reload),//none
    WMEM(set_callback_func),//function memorize_callback, function memorize_chk

    // ex: {enable_encoder=true, max_phrese_length=4}
    Get_WMEM(table),//hash table {key=value..}
    Set_WMEM(table),//hash table {key=value..}
    {"Get_table", raw_get_table},//hash table {key=value..}
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    {"name_space",WRAP(COMPAT<T>::name_space)},//string

    Get_WMEM(enable_charset_filter),//bool
    Get_WMEM(enable_encoder),//bool
    Get_WMEM(enable_sentence),//bool
    Get_WMEM(sentence_over_completion),//bool
    Get_WMEM(encode_commit_history),//int
    Get_WMEM(max_phrase_length),//int
    Get_WMEM(max_homographs),//bool
    // TranslatorOptions
    Get_WMEM(delimiters),//string&
    Get_WMEM(tag),//string
    Get_WMEM(enable_completion),//bool
    Get_WMEM(contextual_suggestions),//bool
    Get_WMEM(strict_spelling),// bool
    Get_WMEM(initial_quality),//double
                              //
    Get_WMEM(preedit_formatter),//Projection&
    Get_WMEM(comment_formatter),//Projection&
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    Set_WMEM(enable_charset_filter),
    Set_WMEM(enable_encoder),
    Set_WMEM(enable_sentence),
    Set_WMEM(sentence_over_completion),
    Set_WMEM(encode_commit_history),
    Set_WMEM(max_phrase_length),
    Set_WMEM(max_homographs),//
    // TranslatorOptions
    Set_WMEM(tag),//string
    Set_WMEM(enable_completion),//bool
    Set_WMEM(contextual_suggestions),//bool
    Set_WMEM(strict_spelling),//bool
    Set_WMEM(initial_quality),//double
    { NULL, NULL },
  };
}// namespace Table_translatorReg
#undef Get_WMEM
#undef WMEM
#undef Set_WMEM

namespace ComponentReg{
  //using LTableTranslator = rime::TableTranslatorReg::LTableTranslator;
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

  template <typename O>
  int raw_make(lua_State *L){
    int n = lua_gettop(L);
    if (3 > n || 4 < n)
      return 0;

    C_State C;
    Ticket ticket(
      LuaType<Engine *>::todata(L, 1),
      LuaType<string>::todata(L, -2, &C),
      LuaType<string>::todata(L, -1, &C)
      );
    if ( n == 4 )
      ticket.schema = &(LuaType<Schema &>::todata(L, 2) ); //overwrite schema
    Lua* lua= Lua::from_state(L);
    //an<O> obj = New<O>(Lua::from_state(L), ticket);
    an<O> obj = New<O>(lua, ticket);
    if (obj) {
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
    {"TableTranslator", raw_make<LTableTranslator>},
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
  EXPORT(TableTranslatorReg, L);
  ComponentReg::init(L);
}
