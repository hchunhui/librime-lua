/*
 * table_translator.cc
 * Copyright (C) 2023 Shewer Lu <shewer@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#include <rime/gear/script_translator.h>
#include <rime/algo/syllabifier.h>
#include <rime/dict/corrector.h>
#include <rime/gear/poet.h>

#include <rime/dict/dictionary.h>
#include <rime/dict/user_dictionary.h>
#include <rime/language.h>

#include "translator.h"

using namespace rime;

namespace {
namespace ScriptTranslatorReg {

  class LScriptTranslator : public ScriptTranslator {
    public:
      LScriptTranslator(const Ticket& ticket, Lua* lua)
        : ScriptTranslator(ticket), lua_(lua) {};

      virtual bool Memorize(const CommitEntry& commit_entry);
      bool memorize(const CommitEntry& commit_entry);
      bool update_entry(const DictEntry& index,
          int commits, const string& new_entory_prefix);

      SET_(memorize_callback, an<LuaObj>);
      optional<an<LuaObj>> memorize_callback();
      string lang_name() const { return (language_) ? language_->name() : "";};

      // TranslatorOptions
      SET_(contextual_suggestions, bool);
      SET_(delimiters, string&);
      SET_(preedit_formatter, Projection&);
      SET_(comment_formatter, Projection&);
      bool reload_user_dict_disabling_patterns(an<ConfigList>);

      // ScriptTranslator member
      ACCESS_(spelling_hints, int);
      ACCESS_(always_show_comments, bool);
      ACCESS_(max_homophones, int);
      GET_(enable_correction, bool);
      void set_enable_correction(bool);

      void disconnect() {
        dict_.reset();
        user_dict_.reset();
        language_.reset();
      }

    protected:
      Lua* lua_;
      an<LuaObj> memorize_callback_;
      void init_correction();
  };

  using T = LScriptTranslator;

  optional<an<LuaObj>> T::memorize_callback() {
    if (memorize_callback_)
      return memorize_callback_;
    return {};
  }

  bool T::memorize(const CommitEntry& commit_entry) {
    return ScriptTranslator::Memorize(commit_entry);
  }

  bool T::Memorize(const CommitEntry& commit_entry) {
    if (!memorize_callback_) {
      return memorize(commit_entry);
    }

    auto r = lua_->call<bool, an<LuaObj>, LScriptTranslator*, const CommitEntry&>(
                   memorize_callback_, this, commit_entry);
    if (!r.ok()) {
      auto e = r.get_err();
      LOG(ERROR) << "LScriptTranslator of " << name_space_
        << ": memorize_callback error(" << e.status << "): " << e.e;
      return false;
    }
    return r.get();
  }

  void T::init_correction() {
    if (auto* corrector = Corrector::Require("corrector")) {
      Ticket ticket(engine_, name_space_);
      corrector_.reset(corrector->Create(ticket));
    }
  }

  void T::set_enable_correction(bool enable) {
    if ((enable_correction_ = enable && !corrector_))
      init_correction();
  }

  bool T::update_entry(const DictEntry& entry,
      int commits, const string& new_entory_prefix) {
    if (user_dict_ && user_dict_->loaded())
      return user_dict_->UpdateEntry(entry, commits, new_entory_prefix);

    return false;
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
    Get_WMEM(name_space),            // string
    Get_WMEM(lang_name),             // string
    Get_WMEM(memorize_callback),     // an<LuaObj> callback function
    // ScriptTranslator member
    Get_WMEM(max_homophones),        // int
    Get_WMEM(spelling_hints),        // int
    Get_WMEM(always_show_comments),  // bool
    Get_WMEM(enable_correction),     // bool
    //TranslatorOptions
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
    // ScriptTranslator member
    Set_WMEM(max_homophones),        // int
    Set_WMEM(spelling_hints),        // int
    Set_WMEM(always_show_comments),  // bool
    Set_WMEM(enable_correction),     // bool
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
      lua_setfield(L, -2, "ScriptTranslator");
    }
    lua_pop(L, 1);
  }

}  // namespace ScriptTranslatorReg
}  // namespace

void LUAWRAPPER_LOCAL script_translator_init(lua_State* L) {
  EXPORT(ScriptTranslatorReg, L);
  ScriptTranslatorReg::reg_Component(L);
}
