/*
 * table_translator.cc
 * Copyright (C) 2023 Shewer Lu <shewer@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#include <rime/gear/table_translator.h>
#include <rime/gear/poet.h>
#include <rime/gear/unity_table_encoder.h>
#include <rime/schema.h>
#include <rime/engine.h>

#include <rime/dict/dictionary.h>
#include <rime/dict/user_dictionary.h>
#include <rime/language.h>

#include "translator.h"

using namespace rime;

namespace {
namespace TableTranslatorReg {

  class LTableTranslator : public TableTranslator {
    public:
      LTableTranslator(const Ticket& ticket, Lua* lua)
        : TableTranslator(ticket), lua_(lua) {};

      virtual bool Memorize(const CommitEntry& commit_entry);
      bool memorize(const CommitEntry& commit_entry);
      bool update_entry(const DictEntry& index,
          int commits, const string& new_entory_prefix);

      SET_(memorize_callback, an<LuaObj>);
      optional<an<LuaObj>> memorize_callback();
      string lang_name() const { return (language_) ? language_->name() : "";};

      // TranslatorOptions
      void set_contextual_suggestions(bool);
      SET_(delimiters, string&);
      SET_(preedit_formatter, Projection&);
      SET_(comment_formatter, Projection&);
      bool reload_user_dict_disabling_patterns(an<ConfigList>);

      // TableTranslator member
      ACCESS_(encode_commit_history, bool);
      ACCESS_(max_phrase_length, int);
      ACCESS_(max_homographs, int);
      ACCESS_(enable_charset_filter, bool);
      GET_(sentence_over_completion, bool);
      void set_sentence_over_completion(bool);
      GET_(enable_encoder, bool);
      void set_enable_encoder(bool);
      GET_(enable_sentence, bool);
      void set_enable_sentence(bool);

      void disconnect() {
        dict_.reset();
        user_dict_.reset();
        language_.reset();
      }

    protected:
      Lua* lua_;
      an<LuaObj> memorize_callback_;
      bool init_poet();
      bool init_encoder();
  };


  using T = LTableTranslator;

  optional<an<LuaObj>> T::memorize_callback() {
    if (memorize_callback_)
      return memorize_callback_;
    return {};
  }

  bool T::memorize(const CommitEntry& commit_entry) {
    return TableTranslator::Memorize(commit_entry);
  }

  bool T::Memorize(const CommitEntry& commit_entry) {
    if (!memorize_callback_) {
      return memorize(commit_entry);
    }

    auto r = lua_->call<bool, an<LuaObj>, LTableTranslator*, const CommitEntry&>(
	           memorize_callback_, this, commit_entry);
    if (!r.ok()) {
      auto e = r.get_err();
      LOG(ERROR) << "LTableTranslator of " << name_space_
		 << ": memorize_callback error(" << e.status << "): " << e.e;
      return false;
    }
    return r.get();
  }

  bool T::update_entry(const DictEntry& entry,
		       int commits, const string& new_entory_prefix) {
    if (user_dict_ && user_dict_->loaded())
      return user_dict_->UpdateEntry(entry, commits, new_entory_prefix);

    return false;
  }

  // enable_encoder
  bool T::init_encoder() {
    if (!user_dict_)
      return false;
    encoder_.reset(new UnityTableEncoder(user_dict_.get()));
    Ticket ticket(engine_, name_space_);
    encoder_->Load(ticket);
    if (!encoder_) {
      LOG(WARNING) << "init encoder failed";
      return false;
    }
    return true;
  }

  void T::set_enable_encoder(bool enable) {
    if ((enable_encoder_ = enable && user_dict_ && !encoder_)) {
      init_encoder();
    }
  }
  // enable sentence contextual
  bool T::init_poet() {
    Config* config = engine_->schema()->config();
    poet_.reset(new Poet(language(), config, Poet::LeftAssociateCompare));
    if (!poet_) {
      LOG(WARNING) << "init poet failed";
      return false;
    }
    return true;
  }

  void T::set_enable_sentence(bool enable) {
    if ((enable_sentence_ = enable && !poet_))
      init_poet();
  }

  void T::set_sentence_over_completion(bool enable) {
    if ((sentence_over_completion_ = enable && !poet_ ))
      init_poet();
  }

  void T::set_contextual_suggestions(bool enable) {
    if ((contextual_suggestions_ = enable && !poet_))
      init_poet();
  }
  bool T::reload_user_dict_disabling_patterns(an<ConfigList> cl) {
    return cl ?  user_dict_disabling_patterns_.Load(cl) : false;
  }

  an<Translator> as_translator(an<T> &t) {
    return As<Translator>(t);
  }

  static const luaL_Reg funcs[] = {
    {NULL, NULL},
  };

  static const luaL_Reg methods[] = {
    {"query", WRAPMEM(T, Query)},  // string, segment
    {"start_session", WRAPMEM(T, StartSession)},
    {"finish_session", WRAPMEM(T, FinishSession)},
    {"discard_session", WRAPMEM(T, DiscardSession)},
    WMEM(memorize),      // delegate TableTransaltor::Momorize
    WMEM(update_entry),  // delegate UserDictionary::UpdateEntry
    WMEM(reload_user_dict_disabling_patterns),
    {"set_memorize_callback", raw_set_memorize_callback<T>},  // an<LuaObj> callback function
    {"disconnect", WRAPMEM(T::disconnect)},
    {NULL, NULL},
  };

  static const luaL_Reg vars_get[] = {
    // class translator member
    Get_WMEM(name_space),                // string
    Get_WMEM(lang_name),                 // string
    Get_WMEM(memorize_callback),         // an<LuaObj> callback function
    // TabletTranslator member
    Get_WMEM(enable_charset_filter),     // bool
    Get_WMEM(enable_encoder),            // bool
    Get_WMEM(enable_sentence),           // bool
    Get_WMEM(sentence_over_completion),  // bool
    Get_WMEM(encode_commit_history),     // int
    Get_WMEM(max_phrase_length),         // int
    Get_WMEM(max_homographs),            // bool
    // TranslatorOptions
    Get_WMEM(delimiters),              // string&
    Get_WMEM(tag),                     // string
    Get_WMEM(enable_completion),       // bool
    Get_WMEM(contextual_suggestions),  // bool
    Get_WMEM(strict_spelling),         // bool
    Get_WMEM(initial_quality),         // double
    Get_WMEM(preedit_formatter),       // Projection&
    Get_WMEM(comment_formatter),       // Projection&
    // Memory
    Get_WMEM(dict),
    Get_WMEM(user_dict),
    {"translator", WRAP(as_translator)},
    {NULL, NULL},
  };

  static const luaL_Reg vars_set[] = {
    {"memorize_callback", raw_set_memorize_callback<T>},  // an<LuaObj> callback function
    // TableTranslator member
    Set_WMEM(enable_charset_filter),     // bool
    Set_WMEM(enable_encoder),            // bool
    Set_WMEM(enable_sentence),           // bool
    Set_WMEM(sentence_over_completion),  // bool
    Set_WMEM(encode_commit_history),     // bool
    Set_WMEM(max_phrase_length),         // int
    Set_WMEM(max_homographs),            // int
    // TranslatorOptions
    Set_WMEM(delimiters),              // string&
    Set_WMEM(tag),                     // string
    Set_WMEM(enable_completion),       // bool
    Set_WMEM(contextual_suggestions),  // bool
    Set_WMEM(strict_spelling),         // bool
    Set_WMEM(initial_quality),         // double
    Set_WMEM(preedit_formatter),       // Projection&
    Set_WMEM(comment_formatter),       // Projection&
    {NULL, NULL},
  };

  void reg_Component(lua_State* L) {
    lua_getglobal(L, "Component");
    if (lua_type(L, -1) != LUA_TTABLE) {
      LOG(ERROR) << "table of _G[\"Component\"] not found.";
    } else {
      lua_pushcfunction(L, raw_make_translator<T>);
      lua_setfield(L, -2, "TableTranslator");
    }
    lua_pop(L, 1);
  }

}  // namespace TableTranslatorReg
}  // namespace

void LUAWRAPPER_LOCAL table_translator_init(lua_State* L) {
  EXPORT(TableTranslatorReg, L);
  TableTranslatorReg::reg_Component(L);
}
