/*
 * translator_options.cc
 * Copyright (C) 2023 Shewer Lu <shewer@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#include <rime/common.h>
#include <rime/language.h>

#include <rime/gear/poet.h>
#include <rime/dict/corrector.h>
#include <rime/gear/script_translator.h>

#include <rime/gear/table_translator.h>
#include <rime/gear/unity_table_encoder.h>

#include "lib/lua_export_type.h"
#include "lib/luatype_boost_optional.h"

#include <utility>

using namespace rime;
namespace {

// Language
namespace LanguageReg {
  typedef rime::Language T;

  the<T> make(const string &s){
    return make_unique<T>(s);
  }

  //string get_language_component(const string &s){
    //return T::get_language_component(s);
  //}

  //string get_language_name(T &t){
    //return t.name();
  //}

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
    //{"Get_language_component",WRAP(T::get_language_component)},
    //{"Get_language_name",WRAP(T,name)},
    //{"Get_language_name",raw_get_language_name},
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
}// end of TranslatorOptions

// ScriptTranslatorOptionsReg
namespace ScriptTranslatorOptionsReg {
  class Corrector;
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
      // create from  user_dictname, dbclass
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
      // create from Ticket (schema , name_spcae)
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
    {"UserDictionary", raw_make}, // UserDictionary(userdict_name, dbtype) ex: "cangjie5","userdb"
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

} // end of namespace

void LUAWRAPPER_LOCAL translator_options_init(lua_State *L) {
  EXPORT_UPTR_TYPE(LanguageReg, L);
  EXPORT(TranslatorOptionsReg, L);
  EXPORT(ScriptTranslatorOptionsReg, L);
  EXPORT(TableTranslatorOptionsReg, L);
  EXPORT_UPTR_TYPE(DictEntryIteratorReg, L);
  EXPORT_UPTR_TYPE(UserDictEntryIteratorReg, L);
  //EXPORT(DictEntryIteratorReg, L);
  EXPORT(DictionaryReg, L);
  EXPORT(UserDictionaryReg, L);
  EXPORT(MemoryRefReg, L);
}

