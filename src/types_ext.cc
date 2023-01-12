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
#include <rime/gear/table_translator.h>
#include <rime/gear/poet.h>
#include <rime/dict/corrector.h>
#include <rime/gear/unity_table_encoder.h>
#include <rime/gear/script_translator.h>
#include <rime/language.h>

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
/*
// Language
namespace LanguageReg {
  typedef rime::Language T;

  an<T> make(const string &s){
    return New<T>(s);
  }
  string get_language_component(const string &s){
    return T::get_language_component(s);
  }
  string get_language_name(T &t){
    return t.name();
  }
  int raw_get_language_name(lua_State *L) {
    T &t = LuaType<T &>::todata(L,1);
    LuaType<string>::pushdata(L, t.name());
    return 1;
  }
  T* get_ptr(T &t) {
    return &t;
  }
  const T* get_cptr(T &t) {
    return &t;
  }

  static const luaL_Reg funcs[] = {
    {"Language", WRAP(make)},
    {"Get_language_component",WRAP(T::get_language_component)},
    //{"Get_language_name",WRAP(T,name)},
    {"Get_language_name",raw_get_language_name},
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    {"name",WRAPMEM(T::name)},
    {"language",WRAP(get_ptr)},
    {"ptr",WRAP(get_ptr)},
    {"cptr",WRAP(get_cptr)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };

}
*/

// TableTranslator of options
namespace TableTranslatorOptionsReg {
  struct TableTranslatorOptions : public TableTranslator {
    bool enable_charset_filter() { return enable_charset_filter_; };
    bool enable_encoder() { return enable_encoder_; };
    bool enable_sentence() { return enable_sentence_; };
    bool sentence_over_completion() { return sentence_over_completion_; };

    bool encode_commit_history() {return encode_commit_history_; };
    int max_phrase_length() { return max_phrase_length_; };
    int max_homographs() { return max_homographs_; };

    void set_enable_charset_filter(bool s) { enable_charset_filter_ = s; };
    void set_enable_encoder(bool s) { enable_encoder_ = s; };
    void set_enable_sentence(bool s) { enable_sentence_ = s; };
    void set_sentence_over_completion(bool s) { sentence_over_completion_ = s; };
    void set_encode_commit_history(bool s) {encode_commit_history_ = s; };
    void set_max_phrase_length(int s) { max_phrase_length_ = s; };
    void set_max_homographs(int s) { max_homographs_ = s; };
  };

  typedef TableTranslatorOptions T;

  static const luaL_Reg funcs[] = {
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    {"enable_charset_filter",WRAPMEM(T, enable_charset_filter)},
    {"enable_encoder",WRAPMEM(T, enable_encoder)},
    {"enable_sentence",WRAPMEM(T, enable_sentence)},
    {"sentence_over_completion",WRAPMEM(T, sentence_over_completion)},
    {"encode_commit_history",WRAPMEM(T, encode_commit_history)},
    {"max_phrase_length",WRAPMEM(T, max_phrase_length)},
    {"max_homographs",WRAPMEM(T, max_homographs)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    {"enable_charset_filter",WRAPMEM(T, set_enable_charset_filter)},
    {"enable_encoder",WRAPMEM(T, set_enable_encoder)},
    {"enable_sentence",WRAPMEM(T, set_enable_sentence)},
    {"sentence_over_completion",WRAPMEM(T, set_sentence_over_completion)},
    {"encode_commit_history",WRAPMEM(T, set_encode_commit_history)},
    {"max_phrase_length",WRAPMEM(T, set_max_phrase_length)},
    {"max_homographs",WRAPMEM(T, set_max_homographs)},
    { NULL, NULL },
  };

}

// ScriptTranslatorOptionsReg
namespace ScriptTranslatorOptionsReg {
  struct ScriptTranslatorOptions : public ScriptTranslator{
    int max_homophones() { return max_homophones_; };
    int spelling_hints() { return spelling_hints_; };
    bool always_show_comment() { return always_show_comments_; };
    bool enable_correction() { return enable_correction_; };
    //the<Corrector> corrector_;
    //the<Poet> poet_;
    void set_max_homophones(int s) { max_homophones_= s; };
    void set_spelling_hints(int s) { spelling_hints_= s; };
    void set_always_show_comment(bool s) { always_show_comments_= s; };
    void set_enable_correction(bool s) { enable_correction_= s; };

  };

  typedef struct ScriptTranslatorOptions T;

  static const luaL_Reg funcs[] = {
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    {"max_homophones",WRAPMEM(T, max_homophones)},
    {"spelling_hints",WRAPMEM(T, spelling_hints)},
    {"always_show_comment",WRAPMEM(T, always_show_comment)},
    {"enable_correction",WRAPMEM(T, enable_correction)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    {"max_homophones",WRAPMEM(T, set_max_homophones)},
    {"spelling_hints",WRAPMEM(T, set_spelling_hints)},
    {"always_show_comment",WRAPMEM(T, set_always_show_comment)},
    {"enable_correction",WRAPMEM(T, set_enable_correction)},
    { NULL, NULL },
  };

}

// TranslatorOptions
namespace TranslatorOptionsReg {
  typedef  TranslatorOptions T;

  static const luaL_Reg funcs[] = {
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    {"is_user_dict_disabled_for",WRAPMEM(T,IsUserDictDisabledFor)},
    {"tag",WRAPMEM(T,tag)},
    {"contextual_suggestions",WRAPMEM(T, contextual_suggestions)},
    {"enable_completion",WRAPMEM(T, enable_completion)},
    {"strict_spelling",WRAPMEM(T, strict_spelling)},
    {"initial_quality",WRAPMEM(T, initial_quality)},
    {"preedit_formatter",WRAPMEM(T, preedit_formatter)},
    {"comment_formatter",WRAPMEM(T, comment_formatter)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    {"tag",WRAPMEM(T,set_tag)},
    {"contextual_suggestions",WRAPMEM(T, set_contextual_suggestions)},
    {"enable_completion",WRAPMEM(T, set_enable_completion)},
    {"strict_spelling",WRAPMEM(T, set_strict_spelling)},
    {"initial_quality",WRAPMEM(T, set_initial_quality)},
    { NULL, NULL },
  };

}
// MemoryRef
namespace MemoryRefReg {
  typedef Memory T;

  string language_name(T &t) {
    return t.language()->name();
  }

  static const luaL_Reg funcs[] = {
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    {"language",WRAPMEM(T,language)},
    {"language_name",WRAP(language_name)},
    {"dict",WRAPMEM(T, dict)},
    {"user_dict",WRAPMEM(T, user_dict)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };

}

namespace TranslatorReg{
  typedef Translator T;
  typedef ScriptTranslatorOptionsReg::ScriptTranslatorOptions SO;
  typedef TableTranslatorOptionsReg::TableTranslatorOptions TO;

  TranslatorOptions* options(T &t ) {
    return dynamic_cast<TranslatorOptions*>(&t);
  }

  Memory* memory(T &t ) {
    return dynamic_cast<Memory*>(&t);
  }

  SO* script_options(T &t){
    if (auto c = dynamic_cast<ScriptTranslator*>(&t))
      return (SO *) c;
    return nullptr;
  }

  const Language* language(T &t) {
    if (auto mem = dynamic_cast<Memory*>(&t) )
      return mem->language();
    else 
      return nullptr;
  }

  TO* table_options(T &t){
    if (auto c = dynamic_cast<TableTranslator*>(&t))
      return (TO *) c;
    return nullptr;
  }
  static const luaL_Reg funcs[] = {
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    {"query",WRAPMEM(T::Query)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    {"name_space",WRAP(COMPAT<T>::name_space)},
    {"options", WRAP(options)},
    {"memory", WRAP(memory)},
    {"language", WRAP(language)},
    {"script_options", WRAP(script_options)},
    {"table_options", WRAP(table_options)},
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
// DictEntryIterator
namespace DictEntryIteratorReg {
  typedef DictEntryIterator T;
  typedef DictEntry DE;
  optional<an<DE>> next(T &t) {
    if (t.exhausted())
      return {};

    auto c = t.Peek();
    t.Next();
    return c;
  }

  int raw_iter(lua_State *L) {
    lua_pushcfunction(L, WRAP(next));
    lua_pushvalue(L, 1);
    return 2;
  }

  T* get_ptr(T& t) {
    return dynamic_cast<T *>(&t);
  }

  static const luaL_Reg funcs[] = {
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    {"Next", WRAPMEM(T::Next)},
    {"Peek", WRAPMEM(T::Peek)},
    {"iter", raw_iter },
    {"next", WRAP(next)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    {"exhausted", WRAPMEM(T::exhausted)},
    {"ptr", WRAP(get_ptr)},
    {"entry_count", WRAPMEM(T::entry_count)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };

}
// UserDictEntryIteratorReg
namespace UserDictEntryIteratorReg {
  typedef UserDictEntryIterator T;
  typedef DictEntry DE;
  optional<an<DE>> next(T &t) {
    if (t.exhausted())
      return {};

    auto c = t.Peek();
    t.Next();
    return c;
  }

  int raw_iter(lua_State *L) {
    lua_pushcfunction(L, WRAP(next));
    lua_pushvalue(L, 1);
    return 2;
  }

  T* get_ptr(T& t) {
    return dynamic_cast<T *>(&t);
  }

  static const luaL_Reg funcs[] = {
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    {"Next", WRAPMEM(T::Next)},
    {"Peek", WRAPMEM(T::Peek)},
    {"iter", raw_iter },
    {"next", WRAP(next)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    {"exhausted", WRAPMEM(T::exhausted)},
    {"ptr", WRAP(get_ptr)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };

}
// Dictionary
namespace DictionaryReg {
  typedef Dictionary T;
  typedef DictEntryIterator ITER;
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
        if (prism.empty())
          prism = dictname;
      }
      else if (3 <= n) {
        dictname = string( lua_tostring(L, 1) );
        prism = string( lua_tostring(L, 2) );
        if (prism.empty())
          prism = dictname;

        for (int i=3; i<=n; i++)
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
      dict->Load();
      LuaType<T *>::pushdata(L, dict);
      return 1;
    };
    return 0;
  }

  Table & get_table(T &t) {
    return (Table &) t.primary_table();
  }

  vector<string> packs(T &t) {
    vector<string> res = t.packs();// clone const vector<string>
    return res;
  }

  the<ITER> lookup_words(T &t, const string &str_code , bool predictive, size_t limit) {
    the<ITER> dict_entry= make_unique<ITER>();
    t.LookupWords(dict_entry.get() , str_code, predictive, limit);
    return std::move(dict_entry);
  };

  //vector<ITER> lookup(T &t, const string &str_code , bool predictive, size_t limit) {
    //vector<ITER> dict_entrys= make_unique<vector<ITER>>();
    //t.Lookup(dict_entry.get() , str_code, predictive, limit);
    //return std::move(dict_entry);
  //};

  static const luaL_Reg funcs[] = {
    {"Dictionary",raw_make},
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    {"load", WRAPMEM(T,Load)},
    {"loaded", WRAPMEM(T,loaded)},
    {"remove", WRAPMEM(T,Remove)},
    {"lookup_words", WRAP(lookup_words)},
    //{"lookup", WRAP(lookup)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    {"name", WRAPMEM(T,name)},
    //{"table", WRAP(get_table)}, // const Table error
    //{"tables", WRAPMEM(T,tables)}, // const vector<table> error
    //{"prism", WRAPMEM(T,prism)},
    {"packs", WRAP(packs)}, // const vector<string> error
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };

}

namespace UserDictionaryReg {
  typedef UserDictionary T;
  typedef Dictionary D;
  typedef UserDictEntryIterator ITER;
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
      dict = c->Create( user_dictname, dbclass);
    }
    else {
      Ticket t = LuaType<Ticket &>::todata(L, 1);
      dict = c->Create(t);
    }

    if ( dict ) {
      dict->Load();
      LuaType<T *>::pushdata(L, dict);
      return 1;
    };
    return 0;
  }

  the<ITER> lookup_words(T &t, const string &str_code , bool predictive, size_t limit) {
    the<ITER> dict_entry= make_unique<ITER>();
    t.LookupWords(dict_entry.get() , str_code, predictive, limit);
    return std::move(dict_entry);
  };

  void attach(T &t, D &d) {
    t.Attach(d.primary_table(), d.prism());
  }

  void unattach(T &t) {
    t.Attach(nullptr, nullptr);
  }

  static const luaL_Reg funcs[] = {
    {"UserDictionary", raw_make}, // UserDictionary("cangjie5","userdb")
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    {"attach", WRAP(attach) }, // udict:attach(dict)
    {"unattach", WRAP(unattach) }, // udict:unattach()
    {"load", WRAPMEM(T, Load)},
    {"loaded", WRAPMEM(T, loaded)},
    {"readonly", WRAPMEM(T, readonly)},
    {"lookup_words", WRAP(lookup_words)},

    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    {"name", WRAPMEM(T, name)},
    {"tick", WRAPMEM(T, tick)},
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
    {"Processor", raw_create<P>},
    {"Segmentor", raw_create<S>},
    {"Translator", raw_create<T>},
    {"Filter", raw_create<F>},
    { NULL, NULL },
  };

  void init(lua_State *L) {
    lua_createtable(L, 0, 0);
    luaL_setfuncs(L, funcs, 0);
    lua_setglobal(L, "Component");
  };

}

} // end of namespace

void LUAWRAPPER_LOCAL types_ext_init(lua_State *L) {
  EXPORT(TicketReg, L);
  //EXPORT(LanguageReg, L);
  EXPORT(ProcessorReg, L);
  EXPORT(SegmentorReg, L);
  EXPORT(TranslatorReg, L);
  EXPORT(FilterReg, L);
  EXPORT(ReverseLookupDictionaryReg, L);
  EXPORT(DbAccessorReg, L);
  EXPORT(LevelDbReg, L);
  EXPORT(TranslatorOptionsReg, L);
  EXPORT(ScriptTranslatorOptionsReg, L);
  EXPORT(TableTranslatorOptionsReg, L);
  EXPORT(MemoryRefReg, L);
  EXPORT(DictionaryReg, L);
  EXPORT(UserDictionaryReg, L);
  EXPORT_UPTR_TYPE(DictEntryIteratorReg, L);
  EXPORT_UPTR_TYPE(UserDictEntryIteratorReg, L);
  ComponentReg::init(L);
}
