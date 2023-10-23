/*
 * table_translator.h
 * Copyright (C) 2023 Shewer Lu <shewer@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef _LUA_TABLE_TRANSLATOR_H
#define _LUA_TABLE_TRANSLATOR_H

#include <rime/gear/table_translator.h>
#include <rime/gear/poet.h>
#include <rime/gear/unity_table_encoder.h>

#include "lib/lua_export_type.h"
void table_translator_init(lua_State* L);

#define SET_(name, type)             \
  void set_##name(const type data) { \
    name##_ = data;                  \
  }
#define GET_(name, type) \
  type name() {          \
    return name##_;      \
  }
#define ACCESS_(name, type) \
  SET_(name, type);         \
  GET_(name, type)

using namespace rime;
class LTableTranslator : public TableTranslator {
 public:
  LTableTranslator(const Ticket& ticket, Lua* lua);
  virtual bool Memorize(const CommitEntry& commit_entry);
  bool memorize(const CommitEntry& commit_entry);
  bool update_entry(const DictEntry& index,
                    int commits,
                    const string& new_entory_prefix);
  void set_disable_userdict(bool);
  GET_(disable_userdict, bool);
  GET_(engine, Engine*);
  ACCESS_(memorize_callback, an<LuaObj>);

  // TableTranslator member
  ACCESS_(enable_charset_filter, bool);  // ok
                                         //
  GET_(enable_encoder, bool);            // ok
  void set_enable_encoder(bool);         // bool
                                  //
  GET_(enable_sentence, bool);
  void set_enable_sentence(bool);
  // ACCESS_(enable_sentence, bool);//ok
  GET_(sentence_over_completion, bool);
  void set_sentence_over_completion(bool);
  // ACCESS_(sentence_over_completion, bool);
  void set_contextual_suggestions(bool);

  ACCESS_(encode_commit_history, bool);
  ACCESS_(max_phrase_length, int);
  ACCESS_(max_homographs, int);

  // TranslatorOptions
  SET_(delimiters, string&)

 protected:
  Lua* lua_;
  an<LuaObj> memorize_callback_ = {};
  bool disable_userdict_ = false;
};
#undef SET_
#undef GET_
#undef ACCESS_
#endif /* !_LUA_TABcd LE_TRANSLATOR_H */
