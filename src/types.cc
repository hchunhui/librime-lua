#include <rime/candidate.h>
#include <rime/translation.h>
#include <rime/segmentation.h>
#include <rime/menu.h>
#include <rime/engine.h>
#include <rime/context.h>
#include <rime/schema.h>
#include <rime/config.h>
#include <rime/gear/translator_commons.h>
#include <rime/dict/reverse_lookup_dictionary.h>
#include <rime/key_event.h>
#include <rime/gear/memory.h>
#include <rime/dict/dictionary.h>
#include <rime/dict/user_dictionary.h>
#include "lua_gears.h"
#include "lib/lua_templates.h"

using namespace rime;

template<typename T>
struct LuaType<optional<T>> {
  static void pushdata(lua_State *L, optional<T> o) {
    if (o)
      LuaType<T>::pushdata(L, *o);
    else
      lua_pushnil(L);
  }

  static optional<T> todata(lua_State *L, int i) {
    if (lua_type(L, i) == LUA_TNIL)
      return {};
    else
      return LuaType<T>::todata(L, i);
  }
};

//--- wrappers for Segment
namespace SegmentReg {
  typedef Segment T;

  T make(int start_pos, int end_pos) {
    return Segment(start_pos, end_pos);
  }

  string get_status(const T &t) {
    switch (t.status) {
    case T::kVoid: return "kVoid";
    case T::kGuess: return "kGuess";
    case T::kSelected: return "kSelected";
    case T::kConfirmed: return "kConfirmed";
    }
    return "";
  }

  void set_status(T &t, const string &r) {
    if (r == "kVoid")
      t.status = T::kVoid;
    else if (r == "kGuess")
      t.status = T::kGuess;
    else if (r == "kSelected")
      t.status = T::kSelected;
    else if (r == "kConfirmed")
      t.status = T::kConfirmed;
  }

  static const luaL_Reg funcs[] = {
    { "Segment", WRAP(make) },
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    { "clear", WRAPMEM(T::Clear) },
    { "close", WRAPMEM(T::Close) },
    { "reopen", WRAPMEM(T::Reopen) },
    { "has_tag", WRAPMEM(T::HasTag) },
    { "get_candidate_at", WRAPMEM(T::GetCandidateAt) },
    { "get_selected_candidate", WRAPMEM(T::GetSelectedCandidate) },
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    { "status", WRAP(get_status) },
    { "start", WRAPMEM_GET(T::start) },
    { "_end", WRAPMEM_GET(T::end) }, // end is keyword in Lua...
    { "length", WRAPMEM_GET(T::length) },
    { "tags", WRAPMEM_GET(T::tags) },
    { "menu", WRAPMEM_GET(T::menu) },
    { "selected_index", WRAPMEM_GET(T::selected_index) },
    { "prompt", WRAPMEM_GET(T::prompt) },
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { "status", WRAP(set_status) },
    { "start", WRAPMEM_SET(T::start) },
    { "_end", WRAPMEM_SET(T::end) }, // end is keyword in Lua...
    { "length", WRAPMEM_SET(T::length) },
    { "tags", WRAPMEM_SET(T::tags) },
    { "menu", WRAPMEM_SET(T::menu) },
    { "selected_index", WRAPMEM_SET(T::selected_index) },
    { "prompt", WRAPMEM_SET(T::prompt) },
    { NULL, NULL },
  };
}

//--- wrappers for an<Candidate>
namespace CandidateReg {
  typedef Candidate T;

  string dynamic_type(T &c) {
    if (dynamic_cast<Phrase *>(&c))
      return "Phrase";
    if (dynamic_cast<SimpleCandidate *>(&c))
      return "Simple";
    if (dynamic_cast<ShadowCandidate *>(&c))
      return "Shadow";
    if (dynamic_cast<UniquifiedCandidate *>(&c))
      return "Uniquified";
    return "Other";
  }

  void set_text(T &c, const string &v) {
    if (auto p = dynamic_cast<SimpleCandidate *>(&c))
      p->set_text(v);
  }

  void set_comment(T &c, const string &v) {
    if (auto p = dynamic_cast<Phrase *>(&c))
      p->set_comment(v);
    else if (auto p = dynamic_cast<SimpleCandidate *>(&c))
      p->set_comment(v);
  }

  void set_preedit(T &c, const string &v) {
    if (auto p = dynamic_cast<Phrase *>(&c))
      p->set_preedit(v);
    else if (auto p = dynamic_cast<SimpleCandidate *>(&c))
      p->set_preedit(v);
  }

  an<T> make(const string type,
                    size_t start, size_t end,
                    const string text, const string comment)
  {
    return New<SimpleCandidate>(type, start, end, text, comment);
  }

  static const luaL_Reg funcs[] = {
    { "Candidate", WRAP(make) },
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    { "get_dynamic_type", WRAP(dynamic_type) },
    { "get_genuine", WRAP(T::GetGenuineCandidate) },
    { "get_genuines", WRAP(T::GetGenuineCandidates) },
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    { "type", WRAPMEM(T::type) },
    { "start", WRAPMEM(T::start) },
    { "_end", WRAPMEM(T::end) }, // end is keyword in Lua...
    { "quality", WRAPMEM(T::quality) },
    { "text", WRAPMEM(T::text) },
    { "comment", WRAPMEM(T::comment) },
    { "preedit", WRAPMEM(T::preedit) },
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { "type", WRAPMEM(T::set_type) },
    { "start", WRAPMEM(T::set_start) },
    { "_end", WRAPMEM(T::set_end) },
    { "quality", WRAPMEM(T::set_quality) },
    { "text", WRAP(set_text) },
    { "comment", WRAP(set_comment) },
    { "preedit", WRAP(set_preedit) },
    { NULL, NULL },
  };
}

//--- wrappers for an<Translation>
namespace TranslationReg {
  typedef Translation T;

  int raw_make(lua_State *L) {
    Lua *lua = Lua::from_state(L);
    int n = lua_gettop(L);

    if (n < 1)
      return 0;

    auto o = lua->newthreadx(L, n);
    an<Translation> r = New<LuaTranslation>(lua, o);
    LuaType<an<Translation>>::pushdata(L, r);
    return 1;
  }

  optional<an<Candidate>> next(T &t) {
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

  static const luaL_Reg funcs[] = {
    { "Translation", raw_make },
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    { "iter", raw_iter },
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };
}

namespace ReverseDbReg {
  typedef ReverseDb T;

  an<T> make(const string &file) {
    an<T> db = New<ReverseDb>(string(RimeGetUserDataDir()) +  "/" + file);
    db->Load();
    return db;
  }

  string lookup(T &db, const string &key) {
    string res;
    if (db.Lookup(key, &res))
      return res;
    else
      return string("");
  }

  static const luaL_Reg funcs[] = {
    { "ReverseDb", WRAP(make) },
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    { "lookup", WRAP(lookup) },
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };
}

namespace SegmentationReg {
  typedef Segmentation T;

  Segment &back(T &t) {
    return t.back();
  }

  void pop_back(T &t) {
    t.pop_back();
  }

  void reset_input(T &t, const string &input) {
    t.Reset(input);
  }

  void reset_length(T &t, const size_t length) {
    t.Reset(length);
  }

  static const luaL_Reg funcs[] = {
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    { "empty", WRAPMEM(T::empty) },
    { "back", WRAP(back) },
    { "pop_back", WRAP(pop_back) },
    { "reset_length", WRAP(reset_length) },
    { "add_segment", WRAPMEM(T::AddSegment) },
    { "forward", WRAPMEM(T::Forward) },
    { "trim", WRAPMEM(T::Trim) },
    { "has_finished_segmentation", WRAPMEM(T::HasFinishedSegmentation) },
    { "get_current_start_position", WRAPMEM(T::GetCurrentStartPosition) },
    { "get_current_end_position", WRAPMEM(T::GetCurrentEndPosition) },
    { "get_current_segment_length", WRAPMEM(T::GetCurrentSegmentLength) },
    { "get_confirmed_position", WRAPMEM(T::GetConfirmedPosition) },
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    { "input", WRAPMEM(T::input) },
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { "input", WRAP(reset_input) },
    { NULL, NULL },
  };
}

namespace MenuReg {
  typedef Menu T;
    
  an<T> make() {
    return New<T>();
  }

  static const luaL_Reg funcs[] = {
    { "Menu", WRAP(make) },
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    { "add_translation", WRAPMEM(T::AddTranslation) },
    //{ }, // AddFilter
    { "prepare", WRAPMEM(T::Prepare) },
    //{ }, // CreatePage
    { "get_candidate_at", WRAPMEM(T::GetCandidateAt) },
    { "candidate_count", WRAPMEM(T::candidate_count) },
    { "empty", WRAPMEM(T::empty) },
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };
}

namespace KeyEventReg {
  typedef KeyEvent T;

  int keycode(const T &t) {
    return t.keycode();
  }

  int modifier(const T &t) {
    return t.modifier();
  }

  static const luaL_Reg funcs[] = {
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    { "shift", WRAPMEM(T::shift) },
    { "ctrl", WRAPMEM(T::ctrl) },
    { "alt", WRAPMEM(T::alt) },
    { "caps", WRAPMEM(T::caps) },
    { "super", WRAPMEM(T::super) },
    { "release", WRAPMEM(T::release) },
    { "repr", WRAPMEM(T::repr) },
    { "eq", WRAPMEM(T::operator==) },
    { "lt", WRAPMEM(T::operator<) },
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    { "keycode", WRAP(keycode) },
    { "modifier", WRAP(modifier) },
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };
}

namespace EngineReg {
  typedef Engine T;

  static const luaL_Reg funcs[] = {
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    { "process_key", WRAPMEM(T::ProcessKey) },
    { "apply_schema", WRAPMEM(T::ApplySchema) },
    { "compose", WRAPMEM(T::Compose)},
    { "commit_text", WRAPMEM(T::CommitText) },
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    { "schema", WRAPMEM(T::schema) },
    { "context", WRAPMEM(T::context) },
    { "active_engine", WRAPMEM(T::active_engine) },
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { "active_engine", WRAPMEM(T::set_active_engine) },
    { NULL, NULL },
  };
}

namespace ContextReg {
  typedef Context T;

  Composition &get_composition(T &t) {
    return t.composition();
  }

  void set_composition(T &t, Composition &c) {
    t.set_composition(std::move(c));
  }

  bool push_input(T &t, const string &str) {
    return t.PushInput(str);
  }

  //CommitHistory &get_commit_history(T &t) {
  //  return t.commit_history();
  //}

  static const luaL_Reg funcs[] = {
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    { "commit", WRAPMEM(T::Commit) },
    { "get_commit_text", WRAPMEM(T::GetCommitText) },
    { "get_script_text", WRAPMEM(T::GetScriptText) },
    { "get_preedit", WRAPMEM(T::GetPreedit) },
    { "is_composing", WRAPMEM(T::IsComposing) },
    { "has_menu", WRAPMEM(T::HasMenu) },
    { "get_selected_candidate", WRAPMEM(T::GetSelectedCandidate) },

    { "push_input", WRAP(push_input) },
    { "pop_input", WRAPMEM(T::PopInput) },
    { "delete_input", WRAPMEM(T::DeleteInput) },
    { "clear", WRAPMEM(T::Clear) },

    { "select", WRAPMEM(T::Select) },
    { "confirm_current_selection", WRAPMEM(T::ConfirmCurrentSelection) },
    { "delete_current_selection", WRAPMEM(T::DeleteCurrentSelection) },
    { "confirm_previous_selection", WRAPMEM(T::ConfirmPreviousSelection) },
    { "reopen_previous_selection", WRAPMEM(T::ReopenPreviousSelection) },
    { "clear_previous_segment", WRAPMEM(T::ClearPreviousSegment) },
    { "reopen_previous_segment", WRAPMEM(T::ReopenPreviousSegment) },
    { "clear_non_confirmed_composition", WRAPMEM(T::ClearNonConfirmedComposition) },
    { "refresh_non_confirmed_composition", WRAPMEM(T::RefreshNonConfirmedComposition) },
    { "set_option", WRAPMEM(T::set_option) },
    { "get_option", WRAPMEM(T::get_option) },
    { "set_property", WRAPMEM(T::set_property) },
    { "get_property", WRAPMEM(T::get_property) },
    { "clear_transient_options", WRAPMEM(T::ClearTransientOptions) },
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    { "composition", WRAP(get_composition) },
    { "input", WRAPMEM(T::input) },
    { "caret_pos", WRAPMEM(T::caret_pos) },
    { "commit_notifier", WRAPMEM(T::commit_notifier) },
    { "select_notifier", WRAPMEM(T::select_notifier) },
    { "update_notifier", WRAPMEM(T::update_notifier) },
    { "delete_notifier", WRAPMEM(T::delete_notifier) },
    { "option_update_notifier", WRAPMEM(T::option_update_notifier) },
    { "property_update_notifier", WRAPMEM(T::property_update_notifier) },
    { "unhandled_key_notifier", WRAPMEM(T::unhandled_key_notifier) },
    //{ "commit_history", WRAP(get_commit_history) },
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { "composition", WRAP(set_composition) },
    { "input", WRAPMEM(T::set_input) },
    { "caret_pos", WRAPMEM(T::set_caret_pos) },
    { NULL, NULL },
  };
}

namespace PreeditReg {
  typedef Preedit T;

  static const luaL_Reg funcs[] = {
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    { "text", WRAPMEM_GET(T::text) },
    { "caret_pos", WRAPMEM_GET(T::caret_pos) },
    { "sel_start", WRAPMEM_GET(T::sel_start) },
    { "sel_end", WRAPMEM_GET(T::sel_end) },
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { "text", WRAPMEM_SET(T::text) },
    { "caret_pos", WRAPMEM_SET(T::caret_pos) },
    { "sel_start", WRAPMEM_SET(T::sel_start) },
    { "sel_end", WRAPMEM_SET(T::sel_end) },
    { NULL, NULL },
  };
}

namespace CompositionReg {
  typedef Composition T;

  an<Segmentation> toSegmentation(T &t) {
    return an<Segmentation>(dynamic_cast<Segmentation*>(&t));
  }
  
  Segment &back(T &t) {
    return t.back();
  }

  void push_back(T &t, Segment &seg) {
    return t.push_back(seg);
  }

  void pop_back(T &t) {
    t.pop_back();
  }

  static const luaL_Reg funcs[] = {
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    { "empty", WRAPMEM(T::empty) },
    { "back", WRAP(back) },
    { "pop_back", WRAP(pop_back) },
    { "push_back", WRAP(push_back) },
    // XXX
    { "has_finished_composition", WRAPMEM(T::HasFinishedComposition) },
    { "get_prompt", WRAPMEM(T::GetPrompt) },
    { "toSegmentation" , WRAP(toSegmentation) },
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };
}

namespace SchemaReg {
  typedef Schema T;
  T* make(string schema_id) {
    return new T(schema_id);
  }

  static const luaL_Reg funcs[] = {
    {"Schema",WRAP(make)},
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    { "schema_id", WRAPMEM(T::schema_id) },
    { "schema_name", WRAPMEM(T::schema_name) },
    { "config", WRAPMEM(T::config) },
    { "page_size", WRAPMEM(T::page_size) },
    { "select_keys", WRAPMEM(T::select_keys) },
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { "config", WRAPMEM(T::set_config) },
    { "select_keys", WRAPMEM(T::set_select_keys) },
    { NULL, NULL },
  };
}

namespace ConfigReg {
  typedef Config T;

  optional<bool> get_bool(T &t, const string &path) {
    bool v;
    if (t.GetBool(path, &v))
      return v;
    else
      return {};
  }

  optional<int> get_int(T &t, const string &path) {
    int v;
    if (t.GetInt(path, &v))
      return v;
    else
      return optional<int>{};
  }

  optional<double> get_double(T &t, const string &path) {
    double v;
    if (t.GetDouble(path, &v))
      return v;
    else
      return optional<double>{};
  }

  optional<string> get_string(T &t, const string &path) {
    string v;
    if (t.GetString(path, &v))
      return v;
    else
      return optional<string>{};
  }

  bool set_string(T &t, const string &path, const string &value) {
    return t.SetString(path, value);
  }

  static const luaL_Reg funcs[] = {
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    //bool LoadFromStream(std::istream& stream);
    //bool SaveToStream(std::ostream& stream);
    { "load_from_file", WRAPMEM(T::LoadFromFile) },
    { "save_to_file", WRAPMEM(T::SaveToFile) },

    { "is_null", WRAPMEM(T::IsNull) },
    { "is_value", WRAPMEM(T::IsValue) },
    { "is_list", WRAPMEM(T::IsList) },
    { "is_map", WRAPMEM(T::IsMap) },
    { "get_bool", WRAP(get_bool) },
    { "get_int", WRAP(get_int) },
    { "get_double", WRAP(get_double) },
    { "get_string", WRAP(get_string) },
    { "get_list_size", WRAPMEM(T::GetListSize) },

    //an<ConfigItem> GetItem(const string& path);
    //an<ConfigValue> GetValue(const string& path);
    //RIME_API an<ConfigList> GetList(const string& path);
    //RIME_API an<ConfigMap> GetMap(const string& path);

    { "set_bool", WRAPMEM(T::SetBool) },
    { "set_int", WRAPMEM(T::SetInt) },
    { "set_double", WRAPMEM(T::SetDouble) },
    { "set_string", WRAP(set_string) },
    //RIME_API bool SetItem(const string& path, an<ConfigItem> item);
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };
}

template<typename T, typename ... I>
static int raw_connect(lua_State *L) {
  Lua *lua = Lua::from_state(L);
  T & t = LuaType<T &>::todata(L, 1);
  an<LuaObj> o = LuaObj::todata(L, 2);

  auto c = t.connect
    ([lua, o](I... i) {
       auto r = lua->void_call<an<LuaObj>, Context *>(o, i...);
       if (!r.ok()) {
         auto e = r.get_err();
         LOG(ERROR) << "Context::Notifier error(" << e.status << "): " << e.e;
       }
     });

  LuaType<boost::signals2::connection>::pushdata(L, c);
  return 1;
}

namespace ConnectionReg {
  typedef boost::signals2::connection T;

  static const luaL_Reg funcs[] = {
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    { "disconnect", WRAPMEM(T::disconnect)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };
}

namespace NotifierReg {
  typedef Context::Notifier T;

  static const luaL_Reg funcs[] = {
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    { "connect", raw_connect<T, Context *>},
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };
}

namespace OptionUpdateNotifierReg {
  typedef Context::OptionUpdateNotifier T;

  static const luaL_Reg funcs[] = {
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    { "connect", raw_connect<T, Context *, const string&>},
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };
}

namespace PropertyUpdateNotifierReg {
  typedef Context::PropertyUpdateNotifier T;

  static const luaL_Reg funcs[] = {
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    { "connect", raw_connect<T, Context *, const string&>},
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };
}

namespace KeyEventNotifierReg {
  typedef Context::KeyEventNotifier T;

  static const luaL_Reg funcs[] = {
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    { "connect", raw_connect<T, Context *, const KeyEvent&>},
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };
}

namespace LogReg {
  void info(const string &s) {
    LOG(INFO) << s;
  }

  void warning(const string &s) {
    LOG(WARNING) << s;
  }

  void error(const string &s) {
    LOG(ERROR) << s;
  }

  static const luaL_Reg funcs[] = {
    { "info", WRAP(info) },
    { "warning", WRAP(warning) },
    { "error", WRAP(error) },
    { NULL, NULL },
  };

  void init(lua_State *L) {
    lua_createtable(L, 0, 0);
    luaL_setfuncs(L, funcs, 0);
    lua_setglobal(L, "log");
  }
}
namespace CommitEntryReg {
  typedef CommitEntry T;

  vector<const rime::DictEntry*> get(T& ce) {
    return ce.elements;
  }

  static const luaL_Reg funcs[] = {
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    {"get",WRAP(get)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };
}
namespace DictEntryReg {
  typedef DictEntry T;
  an<T> make() {
    return an<T>(new T());
  }

  static const luaL_Reg funcs[] = {
    {"DictEntry",WRAP(make)},
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    {"text",WRAPMEM_GET(T::text)},
    {"comment",WRAPMEM_GET(T::comment)},
    {"preedit",WRAPMEM_GET(T::preedit)},
    {"weight",WRAPMEM_GET(T::weight)},
    {"commit_count", WRAPMEM_GET(T::commit_count)},
    {"custom_code",WRAPMEM_GET(T::custom_code)},
    {"remaining_code_length",WRAPMEM_GET(T::remaining_code_length)},
    {"code",WRAPMEM_GET(T::code)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    {"text",WRAPMEM_SET(T::text)},
    {"comment",WRAPMEM_SET(T::comment)},
    {"preedit",WRAPMEM_SET(T::preedit)},
    {"weight",WRAPMEM_SET(T::weight)},
    {"commit_count", WRAPMEM_SET(T::commit_count)},
    {"custom_code",WRAPMEM_SET(T::custom_code)},
    {"remaining_code_length",WRAPMEM_SET(T::remaining_code_length)},
    {"code",WRAPMEM_SET(T::code)},
    { NULL, NULL },
  };
}
namespace CodeReg {

  typedef Code T;

  an<T> make() {
    return an<T>(new Code());
  }

  void pushCode(T& code, const rime::SyllableId inputCode) {
    code.push_back(inputCode);
  }

  string printCode(T& code) {
    return code.ToString();
  }

  static const luaL_Reg funcs[] = {
    {"Code", WRAP(make)},
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    {"push",WRAP(pushCode)},
    {"print",WRAP(printCode)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };
}
namespace MemoryReg {
  class LuaMemory : public Memory {
  public:
    using Memory::Memory;
    virtual bool Memorize(const CommitEntry&);
    int luaCallRef = LUA_REFNIL;
    lua_State* currentState = nullptr;
    DictEntryIterator iter;
    UserDictEntryIterator uter;
    void clearDict() {
      iter = DictEntryIterator();
    }
    void clearUser() {
      uter = UserDictEntryIterator();
    }
  };
  typedef LuaMemory T;

   bool MemoryReg::LuaMemory::Memorize(const CommitEntry& commit_entry) {
    if (!this->currentState) return false;
    lua_State* L = this->currentState;
    lua_rawgeti(L, LUA_REGISTRYINDEX, this->luaCallRef);
    LuaType<CommitEntry>::pushdata(L, static_cast<CommitEntry>(commit_entry));
    int ret = lua_pcall(L, 1, 0, 0);
    if (ret) {
      // didn't figure out how to handle and pass error msg to lua.
      switch (ret)
      {
      case LUA_ERRRUN:
        LOG(ERROR) << (L, "[Memory:Memorize]:LUA_ERRRUN");
        break;
      case LUA_ERRMEM:
        LOG(ERROR) << (L, "[Memory:Memorize]:LUA_ERRMEM");
        break;
      case LUA_ERRERR:
        LOG(ERROR) << (L, "[Memory:Memorize]:LUA_ERRERR");
        break;
      }
    }
    return true;
  }

  an<T> make(Engine* engine, Schema* schema) {
    Ticket translatorTicket;
    translatorTicket.engine = engine;
    translatorTicket.name_space = "translator";
    translatorTicket.schema = schema;
    translatorTicket.klass = "lua_translator";
    an<T> memoli = New<T>(translatorTicket);
    return memoli;
  }

  bool dictLookup(T& memory, const string& input, const bool isExpand,size_t limit) {
    memory.clearDict();
    limit = limit == 0 ? 0xffffffffffffffff : limit;
    return memory.dict()->LookupWords(&memory.iter, input, isExpand, limit) > 0;
  }

  an<T> customMake(Engine* engine,string schema_id, string ns) {
    Ticket ticket;
    ticket.engine = engine;
    if (ns == "") {
      ticket.name_space = "translator";
    } else {
      ticket.name_space = ns;
    }
    Schema schema = Schema(schema_id);
    ticket.schema = &schema;
    ticket.klass = "lua_translator";
    return New<T>(ticket);
  }
  optional<an<DictEntry>> dictNext(T& memory) {
    if (memory.iter.exhausted()) {
      return {};
    }
    an<DictEntry> ret = memory.iter.Peek();
    memory.iter.Next();
    return ret;
  }

  bool userLookup(T& memory, const string& input, const bool isExpand) {
    memory.clearUser();
    return memory.user_dict()->LookupWords(&memory.uter, input, isExpand) > 0;
  }
  optional<an<DictEntry>> userNext(T& memory) {
    if (memory.uter.exhausted()) {
      return {};
    }
    an<DictEntry> ret = memory.uter.Peek();
    memory.uter.Next();
    return ret;
  }

  bool updateToUserdict(T& memory, const DictEntry& entry, const int commits, const string& new_entry_prefix) {
    return memory.user_dict()->UpdateEntry(entry, commits, new_entry_prefix);
  }

  int raw_iter_user(lua_State* L) {
    lua_pushcfunction(L, WRAP(userNext));
    lua_pushvalue(L, 1);
    return 2;
  }

  int raw_iter_dict(lua_State* L) {
    lua_pushcfunction(L, WRAP(dictNext));
    lua_pushvalue(L, 1);
    return 2;
  }

  int memorize(lua_State* L) {
    // object itself & passed function
    if (lua_gettop(L) == 2)
    {
      if (!lua_isfunction(L, -1)) {
        const char* msg = lua_pushfstring(L, "%s expected, pass function please", lua_typename(L, lua_type(L, -1)));
        luaL_argerror(L, 2, msg);
      } else {
        an<T> memory = LuaType<an<T>>::todata(L, 1);
        luaL_unref(L, LUA_REGISTRYINDEX, memory->luaCallRef);
        memory->luaCallRef = luaL_ref(L, LUA_REGISTRYINDEX);
        memory->currentState = L;
        lua_settop(L, 0);
      }
    }
    return 0;
  }

  static const luaL_Reg funcs[] = {
      {"Memory", WRAP(make)},
      {"CustomMemory",WRAP(customMake)},
      {NULL, NULL},
  };

  std::vector<string> decode(T& memory, Code& code) {
    std::vector<string> res;
    memory.dict()->Decode(code,&res);
    return res;
  }
  static const luaL_Reg methods[] = {
      { "dict_lookup", WRAP(dictLookup)},
      { "user_lookup", WRAP(userLookup)},
      { "memorize", memorize},
      { "decode", WRAP(decode)},
      { "iter_dict", raw_iter_dict},
      { "iter_user", raw_iter_user},
      { "update_userdict", WRAP(updateToUserdict)},
      {NULL, NULL},
  };

  static const luaL_Reg vars_get[] = {
      {NULL, NULL},
  };

  static const luaL_Reg vars_set[] = {
      {NULL, NULL},
  };
}  // namespace MemoryReg

//--- wrappers for Phrase
namespace PhraseReg {
  typedef Phrase T;

  an<T> make(MemoryReg::LuaMemory& memory, 
    const string& type,
    size_t start,
    size_t end,
    const an<DictEntry>& entry)
  {
    return New<Phrase>(memory.language(),type, start,end, entry);
  }

  an<Candidate> toCandidate(an<T> phrase) {
    return phrase;
  }

  static const luaL_Reg funcs[] = {
    { "Phrase", WRAP(make) },
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    { "toCandidate", WRAP(toCandidate)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    { "language", WRAPMEM(T::language)},
    { "type", WRAPMEM(T::type) },
    { "start", WRAPMEM(T::start) },
    { "_end", WRAPMEM(T::end) }, // end is keyword in Lua...
    { "quality", WRAPMEM(T::quality) },
    { "text", WRAPMEM(T::text) },
    { "comment", WRAPMEM(T::comment) },
    { "preedit", WRAPMEM(T::preedit) },
    { "weight", WRAPMEM(T::weight)},
    { "code", WRAPMEM(T::code)},
    { "entry", WRAPMEM(T::entry)},
    //span
    //language doesn't wrap yet, so Wrap it later
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { "type", WRAPMEM(T::set_type) },
    { "start", WRAPMEM(T::set_start) },
    { "_end", WRAPMEM(T::set_end) },
    { "quality", WRAPMEM(T::set_quality) },
    { "comment", WRAPMEM(T::set_comment) },
    { "preedit", WRAPMEM(T::set_preedit) },
    { "weight", WRAPMEM(T::set_weight)},
    // set_syllabifier
    { NULL, NULL },
  };
}// Phrase work with Translator

namespace KeySequenceReg {
  typedef KeySequence T;

  an<T> make() {
    return New<T>();
  }

  vector<KeyEvent> toKeyEvent(T& t) {
    return t;
  }

  static const luaL_Reg funcs[] = {
    { "KeySequence", WRAP(make) },
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    { "parse", WRAPMEM(T::Parse) },
    { "toKeyEvent", WRAP(toKeyEvent) },
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };
}// KeySequence a vector of Keyevent

//--- Lua
#define EXPORT(ns, L) \
  do { \
  export_type(L, LuaType<ns::T>::name(), LuaType<ns::T>::gc,       \
              ns::funcs, ns::methods, ns::vars_get, ns::vars_set); \
  export_type(L, LuaType<ns::T &>::name(), NULL,                   \
              ns::funcs, ns::methods, ns::vars_get, ns::vars_set); \
  export_type(L, LuaType<const ns::T>::name(), LuaType<ns::T>::gc, \
              ns::funcs, ns::methods, ns::vars_get, ns::vars_set); \
  export_type(L, LuaType<const ns::T &>::name(), NULL,             \
              ns::funcs, ns::methods, ns::vars_get, ns::vars_set); \
  export_type(L, LuaType<an<ns::T>>::name(), LuaType<an<ns::T>>::gc, \
              ns::funcs, ns::methods, ns::vars_get, ns::vars_set); \
  export_type(L, LuaType<an<const ns::T>>::name(), LuaType<an<const ns::T>>::gc, \
              ns::funcs, ns::methods, ns::vars_get, ns::vars_set); \
  export_type(L, LuaType<ns::T *>::name(), NULL,                   \
              ns::funcs, ns::methods, ns::vars_get, ns::vars_set); \
  export_type(L, LuaType<const ns::T *>::name(), NULL,             \
              ns::funcs, ns::methods, ns::vars_get, ns::vars_set); \
  } while (0)

void export_type(lua_State *L,
                 const char *name, lua_CFunction gc,
                 const luaL_Reg *funcs, const luaL_Reg *methods,
                 const luaL_Reg *vars_get, const luaL_Reg *vars_set);

void types_init(lua_State *L) {
  EXPORT(SegmentReg, L);
  EXPORT(CandidateReg, L);
  EXPORT(TranslationReg, L);
  EXPORT(ReverseDbReg, L);
  EXPORT(SegmentationReg, L);
  EXPORT(MenuReg, L);
  EXPORT(KeyEventReg, L);
  EXPORT(EngineReg, L);
  EXPORT(ContextReg, L);
  EXPORT(PreeditReg, L);
  EXPORT(CompositionReg, L);
  EXPORT(SchemaReg, L);
  EXPORT(ConfigReg, L);
  EXPORT(NotifierReg, L);
  EXPORT(OptionUpdateNotifierReg, L);
  EXPORT(PropertyUpdateNotifierReg, L);
  EXPORT(KeyEventNotifierReg, L);
  EXPORT(ConnectionReg, L);
  EXPORT(MemoryReg, L);
  EXPORT(DictEntryReg, L);
  EXPORT(CodeReg, L);
  EXPORT(CommitEntryReg, L);
  EXPORT(PhraseReg, L);
  EXPORT(KeySequenceReg, L);
  LogReg::init(L);
}
