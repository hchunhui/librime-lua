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

#include <boost/regex.hpp>

#include "translator.h"
namespace {

using namespace rime;

class LScriptTranslator : public ScriptTranslator {
 public:
  LScriptTranslator(const Ticket& ticket, Lua* lua);
  virtual bool Memorize(const CommitEntry& commit_entry);
  bool memorize(const CommitEntry& commit_entry);
  bool update_entry(const DictEntry& index,
                    int commits,
                    const string& new_entory_prefix);
  GET_(engine, Engine*);
  ACCESS_(memorize_callback, an<LuaObj>);

  // ScriptTranslator member
  ACCESS_(spelling_hints, int);         // ok
  ACCESS_(always_show_comments, bool);  // ok
  ACCESS_(max_homophones, int);         // ok
  GET_(enable_correction, bool);        // ok
  void set_enable_correction(bool);
  // TranslatorOptions
  void set_contextual_suggestions(bool);
  SET_(delimiters, string&);

 protected:
  Lua* lua_;
  an<LuaObj> memorize_callback_;
  string schema_id_;
};

bool LScriptTranslator::Memorize(const CommitEntry& commit_entry) {
  if (!memorize_callback_ || memorize_callback_->type() != LUA_TFUNCTION) {
    return ScriptTranslator::Memorize(commit_entry);
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

bool LScriptTranslator::memorize(const CommitEntry& commit_entry) {
  return ScriptTranslator::Memorize(commit_entry);
}

LScriptTranslator::LScriptTranslator(const Ticket& ticket, Lua* lua)
    : lua_(lua),
      ScriptTranslator(ticket),
      schema_id_(ticket.schema->schema_id()) {
  memorize_callback_ = lua_->getglobal("___");
}

void LScriptTranslator::set_enable_correction(bool enable) {
  enable_correction_ = enable;
  if (corrector_ || !enable_correction_)
    return;

  if (auto* corrector = Corrector::Require("corrector")) {
    Schema schema = Schema(schema_id_);
    Ticket ticket(&schema, name_space_);
    corrector_.reset(corrector->Create(ticket));
  }
}
void LScriptTranslator::set_contextual_suggestions(bool enable) {
  contextual_suggestions_ = enable;
  if (poet_ || !contextual_suggestions_)
    return;
  Config* config = engine_->schema()->config();
  poet_.reset(new Poet(language(), config, Poet::LeftAssociateCompare));
}

bool LScriptTranslator::update_entry(const DictEntry& entry,
                                     int commits,
                                     const string& new_entory_prefix) {
  if (user_dict_ && user_dict_->loaded())
    return user_dict_->UpdateEntry(entry, commits, new_entory_prefix);

  return false;
}

namespace ScriptTranslatorReg {
using T = LScriptTranslator;

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
    {NULL, NULL},
};

static const luaL_Reg vars_get[] = {
    Get_WMEM(engine),      // engine*
    Get_WMEM(memorize_callback),  // an<LuaObj> callback

    // ScriptTranslator member
    Get_WMEM(max_homophones),        // int
    Get_WMEM(spelling_hints),        // int
    Get_WMEM(always_show_comments),  // bool
    Get_WMEM(enable_correction),     // bool
    // TranslatorOptions
    Get_WMEM(delimiters),              // string&
    Get_WMEM(tag),                     // string
    Get_WMEM(enable_completion),       // bool
    Get_WMEM(contextual_suggestions),  // bool
    Get_WMEM(strict_spelling),         // bool
    Get_WMEM(initial_quality),         // double
    // Memory
    { "dict", WRAPMEM(T, dict)},
    { "user_dict", WRAPMEM(T, user_dict)},
    {NULL, NULL},
};

static const luaL_Reg vars_set[] = {
    Set_WMEM(memorize_callback),  // an<LuaObj> callback

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
    {NULL, NULL},
};

void reg_Component(lua_State* L) {
  lua_getglobal(L, "Component");
  if (lua_type(L, -1) != LUA_TTABLE) {
    LOG(ERROR) << "table of _G[\"Component\"] not found.";
  } else {
    int index = lua_absindex(L, -1);
    lua_pushcfunction(L, raw_make_translator<LScriptTranslator>);
    lua_setfield(L, index, "ScriptTranslator");
  }
  lua_pop(L, 1);
}

}  // namespace ScriptTranslatorReg
}  // namespace

void LUAWRAPPER_LOCAL script_translator_init(lua_State* L) {
  EXPORT(ScriptTranslatorReg, L);
  ScriptTranslatorReg::reg_Component(L);
}
