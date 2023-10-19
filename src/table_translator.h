/*
 * table_translator.h
 * Copyright (C) 2023 Shewer Lu <shewer@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef _LUA_TABLE_TRANSLATOR_H
#define _LUA_TABLE_TRANSLATOR_H
#include <rime/common.h>
#include <rime/gear/table_translator.h>

//#include <rime/gear/poet.h>
//#include <rime/gear/unity_table_encoder.h>

#include "lib/lua_export_type.h"
#include "lib/luatype_boost_optional.h"

#define SET_(name, type) void set_##name(const type data) { name##_ = data;}
#define GET_(name, type) type name() { return name##_;}
#define ACCESS_(name, type) \
  SET_(name, type);\
  GET_(name, type)


using namespace rime;
//namespace {
//namespace TableTranslatorReg {

using CommitEntry = rime::CommitEntry;

class LTableTranslator : public TableTranslator {
  public:
    LTableTranslator(Lua *lua, const Ticket& ticket);
    virtual bool Memorize(const CommitEntry& commit_entry);
    
    void set_callback_func(an<LuaObj> memorize_callback, an<LuaObj> memorize_chk);
    void reload();
    void set_table(an<LuaObj>);
    an<LuaObj> table();


    ACCESS_(enable_charset_filter, bool);
    //bool enable_charset_filter() {return  enable_charset_filter_;}
    //void set_enable_charset_filter(bool en) { enable_charset_filter_= en;}
    ACCESS_(enable_encoder, bool);
    //bool enable_encoder() { return enable_encoder_;}
    //void set_enable_encoder(bool en) { enable_encoder_ = en;}
    ACCESS_(enable_sentence, bool);
    //bool enable_sentence() { return enable_sentence_;}
    //void set_enable_sentence(bool en) { enable_sentence_ = en;}
    ACCESS_(sentence_over_completion, bool);
    //bool sentence_over_completion() { return sentence_over_completion_;}
    //void set_sentence_over_completion(bool en) { sentence_over_completion_ = en;}
    ACCESS_(encode_commit_history, bool);
    //bool encode_commit_history() { return encode_commit_history_;}
    //void set_encode_commit_history(bool en) { encode_commit_history_ = en;}
    ACCESS_(max_phrase_length, int);
    //int max_phrase_length() { return max_phrase_length_;}
    //void set_max_phrase_length(const int length) { max_phrase_length_ =length;}
    ACCESS_(max_homographs, int);
    //int max_homographs() { return  max_homographs_;}
    //void set_max_homographs(const int length) { max_homographs_ = length;}

    //TranslatorOptions

  protected:
    Lua* lua_;
    an<LuaObj> memorize_callback_;
    an<LuaObj> memorize_chk_func_;
};
#undef SET_
#undef GET_
#undef ACCESS_

//}//TableTranslatorReg
//}//namespace
#endif /* !_LUA_TABcd LE_TRANSLATOR_H */
