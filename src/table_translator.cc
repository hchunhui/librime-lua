#include "table_translator.h"
#include <rime/dict/dictionary.h>
#include <rime/dict/user_dictionary.h>
#include <rime/gear/memory.h>
#include <rime/ticket.h>
#include <rime/schema.h>
#include <rime/engine.h>
#include <rime/config.h>
#include <rime/gear/poet.h>
#include <rime/gear/unity_table_encoder.h>


//#include <rime/context.h>
using namespace rime;
//namespace {
//namespace TableTranslatorReg {

LTableTranslator::LTableTranslator(Lua* lua, const Ticket& ticket)
  : lua_(lua), TableTranslator(ticket) { };

void LTableTranslator::set_callback_func(an<LuaObj> memorize_callback, an<LuaObj> memorize_chk) {
}

void LTableTranslator::reload() {
   //copy from TableTranslator::TableTranslator(ticket)
  if (!engine_)
    return;

  if (Config* config = engine_->schema()->config()) {
    config->GetBool(name_space_ + "/enable_charset_filter",
                    &enable_charset_filter_);
    config->GetBool(name_space_ + "/enable_sentence", &enable_sentence_);
    config->GetBool(name_space_ + "/sentence_over_completion",
                    &sentence_over_completion_);
    config->GetBool(name_space_ + "/enable_encoder", &enable_encoder_);
    config->GetBool(name_space_ + "/encode_commit_history",
                    &encode_commit_history_);
    config->GetInt(name_space_ + "/max_phrase_length", &max_phrase_length_);
    config->GetInt(name_space_ + "/max_homographs", &max_homographs_);
    if (enable_sentence_ || sentence_over_completion_ ||
        contextual_suggestions_) {
      poet_.reset(new Poet(language(), config, Poet::LeftAssociateCompare));
    }
  }
  if (enable_encoder_ && user_dict_) {
    Ticket ticket(engine_, name_space_);
    encoder_.reset(new UnityTableEncoder(user_dict_.get()));
    encoder_->Load(ticket);
  }
}

// memorize_chk_func(self, const Selfcommit_entry)
bool LTableTranslator::Memorize(const CommitEntry& commit_entry) {
    bool memorize_chk = true;
    if (memorize_chk_func_) {
      auto r =lua_->call<bool, an<LuaObj>, LTableTranslator*, const CommitEntry& >(
          memorize_chk_func_, this, commit_entry);
      if (!r.ok()) {
        auto e = r.get_err();
        LOG(ERROR) << "LTableTranslator is_memorize of " << name_space_
          << "error(" << e.status << "): " << e.e;
        memorize_chk = true;
      } 
      else {
        memorize_chk = r.get();
      }
    }
    if (!memorize_chk) 
      return false;

    // 
    if (!memorize_callback_) {
      return TableTranslator::Memorize(commit_entry);
    }

    auto r =lua_->call<bool, an<LuaObj>, LTableTranslator*, const CommitEntry& >(
            memorize_callback_, this, commit_entry);
    if (!r.ok()) {
      auto e = r.get_err();
      LOG(ERROR) << "LTableTranslator memorize_callback of " << name_space_
        << "error(" << e.status << "): " << e.e;
      return false;
    } 
    return r.get();
}

void LTableTranslator::set_table(an<LuaObj>) {

}

an<LuaObj> LTableTranslator::table() {
  an<LuaObj> table;
  lua_->to_state(
      [&](lua_State *L) {

      });
  return table;
}

//}//TableTranslatorReg
//}// namespace
//}//namespace
