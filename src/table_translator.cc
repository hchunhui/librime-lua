#include "table_translator.h"
#include <rime/schema.h>
#include <rime/engine.h>

#include <boost/regex.hpp>
#include "lib/lua_export_type.h"
#include "lib/luatype_boost_optional.h"

bool LTableTranslator::Memorize(const CommitEntry& commit_entry) {
  if (!memorize_callback_ || memorize_callback_->type() != LUA_TFUNCTION) {
    return TableTranslator::Memorize(commit_entry);
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

bool LTableTranslator::update_entry(const DictEntry& entry,
                                    int commits,
                                    const string& new_entory_prefix) {
  if (user_dict_ && user_dict_->loaded())
    return user_dict_->UpdateEntry(entry, commits, new_entory_prefix);

  return false;
}

bool LTableTranslator::memorize(const CommitEntry& commit_entry) {
  return TableTranslator::Memorize(commit_entry);
}

LTableTranslator::LTableTranslator(const Ticket& ticket, Lua* lua)
    : lua_(lua), TableTranslator(ticket) {
  bool disable_userdict;
  Config* config = ticket.schema->config();
  config->GetBool(name_space_ + "/disable_userdict", &disable_userdict);
  set_disable_userdict(disable_userdict);
}
void LTableTranslator::set_enable_encoder(bool enable_encoder) {
  enable_encoder_ = enable_encoder;
  if (encoder_ || !user_dict_ || !enable_encoder_)
    return;
  // create   "__" ns "__/dictionary"  for lLoad()
  // load encode
  encoder_.reset(new UnityTableEncoder(user_dict_.get()));
  Schema* schema = engine_->schema();
  Config* config = schema->config();
  string ns = "__" + name_space_ + "__";
  config->SetString(ns + "/dictionary", dict_->name());
  Ticket ticket(schema, ns);
  encoder_->Load(ticket);
}

void LTableTranslator::set_enable_sentence(bool enable) {
  enable_sentence_ = enable;
  if (poet_ || !enable_sentence_)
    return;
  Config* config = engine_->schema()->config();
  poet_.reset(new Poet(language(), config, Poet::LeftAssociateCompare));
}
void LTableTranslator::set_sentence_over_completion(bool enable) {
  sentence_over_completion_ = enable;
  if (poet_ || !sentence_over_completion_)
    return;
  Config* config = engine_->schema()->config();
  poet_.reset(new Poet(language(), config, Poet::LeftAssociateCompare));
}
void LTableTranslator::set_contextual_suggestions(bool enable) {
  contextual_suggestions_ = enable;
  if (poet_ || !contextual_suggestions_)
    return;
  Config* config = engine_->schema()->config();
  poet_.reset(new Poet(language(), config, Poet::LeftAssociateCompare));
}

static boost::regex ensure_disable_pattern_ = boost::regex(".*");
void LTableTranslator::set_disable_userdict(bool disable_userdict) {
  if (disable_userdict_ == disable_userdict)
    return;
  disable_userdict_ = disable_userdict;
  if (disable_userdict)
    user_dict_disabling_patterns_.push_back(ensure_disable_pattern_);
  else
    user_dict_disabling_patterns_.pop_back();
}

template <typename I>
void pushkeyvalue(lua_State* L, int index, const string& key, I& value) {
  LuaType<I>::pushdata(L, value);
  lua_setfield(L, index - 1, key.c_str());
};

#define Set_WMEM(name) \
  { #name, WRAPMEM(T, set_##name) }
#define WMEM(name) \
  { #name, WRAPMEM(T, name) }
#define Get_WMEM(name) \
  { #name, WRAPMEM(T, name) }

#if LUA_VERSION_NUM < 502
#define lua_absindex(L, i) abs_index((L), (i))
#endif
namespace {
namespace TableTranslatorReg {
using T = LTableTranslator;

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
    // class translator member
    Get_WMEM(engine),      // engine*
    Get_WMEM(name_space),  // string
    // TabletTranslator member
    Get_WMEM(disable_userdict),          // bool
    Get_WMEM(memorize_callback),         // an<LuaObj> callback
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
    {NULL, NULL},
};

static const luaL_Reg vars_set[] = {
    Set_WMEM(disable_userdict),   // bool
    Set_WMEM(memorize_callback),  // an<LuaObj> callback

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
    {NULL, NULL},
};
#undef Get_WMEM
#undef WMEM
#undef Set_WMEM
}  // namespace TableTranslatorReg

}  // namespace

void LUAWRAPPER_LOCAL table_translator_init(lua_State* L) {
  EXPORT(TableTranslatorReg, L);
}
