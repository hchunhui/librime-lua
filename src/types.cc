#include <rime/candidate.h>
#include <rime/translation.h>
#include <rime/segmentation.h>
#include <rime/menu.h>
#include <rime/engine.h>
#include <rime/context.h>
#include <rime/schema.h>
#include <rime/config.h>
#include <rime/config/config_component.h>
#include <rime/config/config_types.h>
#include <rime/gear/translator_commons.h>
#include <rime/dict/reverse_lookup_dictionary.h>
#include <rime/key_event.h>
#include <rime/switcher.h>
#include "lua_gears.h"
#include "lib/lua_templates.h"
#include <opencc/opencc.h>
#include "lua_ext_gears.h"

using namespace rime;

template<typename T>
struct LuaType<optional<T>> {
  static void pushdata(lua_State *L, optional<T> o) {
    if (o)
      LuaType<T>::pushdata(L, *o);
    else
      lua_pushnil(L);
  }

  static optional<T> &todata(lua_State *L, int i, C_State *C) {
    if (lua_type(L, i) == LUA_TNIL)
      return C->alloc<optional<T>>();
    else
      return C->alloc<optional<T>>(LuaType<T>::todata(L, i, C));
  }
};

//--- wrappers for Segment
namespace SegmentReg {
  typedef Segment T;

  T make(int start_pos, int end_pos) {
    return Segment(start_pos, end_pos);
  };

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

  bool empty(T &t){
    return t.empty();
  }

  static const luaL_Reg funcs[] = {
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    { "empty", WRAP(empty) },
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

  an<T> make(const string &key) {
    return New<T>(key) ;
  }

  static const luaL_Reg funcs[] = {
    { "KeyEvent", WRAP(make)  },
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

  static void apply_schema(T *engine, the<Schema> &schema) {
    engine->ApplySchema(schema.release());
  }

  static const luaL_Reg funcs[] = {
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    { "process_key", WRAPMEM(T::ProcessKey) },
    { "compose", WRAPMEM(T::Compose)},
    { "commit_text", WRAPMEM(T::CommitText) },
    { "apply_schema", WRAP(apply_schema) },
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

  Segmentation *toSegmentation(T &t) {
    return dynamic_cast<Segmentation *>(&t);
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

  bool empty(T &t){
    return t.empty();
  }

  static const luaL_Reg funcs[] = {
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    { "empty", WRAP(empty) },
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

  the<T> make(const string &schema_id) {
    return std::unique_ptr<T>(new T(schema_id));
  };

  static const luaL_Reg funcs[] = {
    { "Schema", WRAP(make) },
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

namespace ConfigValueReg {
  typedef ConfigValue T;
  typedef ConfigItem E;

  // an<T> make(){
  //  return New<T>();
  // };
  an<T> make(string s){
    return New<T>(s);
  };

  optional<bool> get_bool(T &t) {
    bool v;
    if (t.GetBool( &v))
      return v;
    else
      return {};
  }

  optional<int> get_int(T &t) {
    int v;
    if (t.GetInt( &v))
      return v;
    else
      return optional<int>{};
  }

  optional<double> get_double(T &t) {
    double v;
    if (t.GetDouble( &v))
      return v;
    else
      return optional<double>{};
  }

  optional<string> get_string(T &t) {
    string v;
    if (t.GetString( &v))
      return v;
    else
      return optional<string>{};
  }

  bool set_string(T &t, const string &value) {
    return t.SetString( value);
  }

  string type(T &t){
    switch (t.type()) {
    case T::kNull: return "kNull";
    case T::kScalar: return "kScalar";
    case T::kList: return "kList";
    case T::kMap: return "kMap";
    }
    return "";
  }

  an<E> element(an<T> t){
    return t ;
  }

  static const luaL_Reg funcs[] = {
    {"ConfigValue", WRAP(make)},
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    {"get_bool",WRAP(get_bool)},
    {"get_int",WRAP(get_int)},
    {"get_double",WRAP(get_double)},
    {"set_bool", WRAPMEM(T::SetBool)},
    {"set_int", WRAPMEM(T::SetInt)},
    {"set_double", WRAPMEM(T::SetDouble)},
    {"get_string",WRAP(get_string)},
    {"set_string",WRAP(set_string)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    {"value",WRAP(get_string)},
    {"type",WRAP(type)},
    {"element",WRAP(element)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    {"value",WRAP(set_string)},
    { NULL, NULL },
  };
}
namespace ConfigListReg {
  typedef ConfigList T;
  typedef ConfigItem E;

  an<T> make(){
    return New<T>();
  };

  string type(T &t){
    switch (t.type()) {
    case T::kNull: return "kNull";
    case T::kScalar: return "kScalar";
    case T::kList: return "kList";
    case T::kMap: return "kMap";
    }
    return "";
  }

  an<E> element(an<T> t){
    return t;
  }

  static const luaL_Reg funcs[] = {
    {"ConfigList", WRAP(make)},
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    {"get_at", WRAPMEM(T::GetAt)},
    {"get_value_at", WRAPMEM(T::GetValueAt)},
    {"set_at", WRAPMEM(T::SetAt)},
    {"append", WRAPMEM(T::Append)},
    {"insert", WRAPMEM(T::Insert)},
    {"clear", WRAPMEM(T::Clear)},
    {"empty", WRAPMEM(T::empty)},
    {"resize", WRAPMEM(T::Resize)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    {"size", WRAPMEM(T::size)},
    {"type",WRAP(type)},
    {"element",WRAP(element)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };
}


namespace ConfigMapReg {
  typedef ConfigMap T;
  typedef ConfigItem E;

  an<T> make(){
    return New<T>();
  }

  string type(T &t){
    switch (t.type()) {
    case T::kNull: return "kNull";
    case T::kScalar: return "kScalar";
    case T::kList: return "kList";
    case T::kMap: return "kMap";
    }
    return "";
  }

  size_t size(T &t){
    size_t count=0;
    for (auto it=t.begin(); it !=t.end();it++)
      count++ ;
    return count;
  }

  an<E> element(an<T> t){
    return t ;
  }

  int raw_keys(lua_State *L) {
    int n = lua_gettop(L);
    if ( n < 1 ) return 0;
    an<T> t= LuaType<an<T>>::todata(L, 1);
    lua_pop(L, n);
    lua_newtable(L);
    int index=1;
    for ( auto  it : *t) {
      lua_pushstring(L, it.first.c_str());
      lua_seti(L,1 , index++);
    }
    return 1;
  }


  static const luaL_Reg funcs[] = {
    {"ConfigMap", WRAP(make)},
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    {"set", WRAPMEM(T::Set)},
    {"get", WRAPMEM(T::Get)},
    {"get_value", WRAPMEM(T::GetValue)},
    {"has_key", WRAPMEM(T::HasKey)},
    {"clear", WRAPMEM(T::Clear)},
    {"empty", WRAPMEM(T::empty)},
    {"keys", raw_keys},
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    {"size", WRAP(size)},
    {"type",WRAP(type)},
    {"element",WRAP(element)},
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };
}

namespace ConfigItemReg {
  typedef ConfigItem T;
  typedef ConfigMap M;
  typedef ConfigList L;
  typedef ConfigValue V;

  string type(T &t){
    switch (t.type()) {
    case T::kNull: return "kNull";
    case T::kScalar: return "kScalar";
    case T::kList: return "kList";
    case T::kMap: return "kMap";
    }
    return "";
  }

//START_GET_
//sed  sed -n -e'/\/\/START_GET_/,/\/\/END_GET_/p' src/types.cc | gcc -E -
#define GET_(f_name,from ,rt, k_type) \
  an<rt> f_name( an<from> t) { \
    if (t->type() == from::k_type) \
      return std::dynamic_pointer_cast<rt> (t);\
    return nullptr;\
  }

  GET_( get_value,T,  V, kScalar );
  GET_( get_list, T,  L, kList );
  GET_( get_map,  T,  M, kMap );

#undef GET_
//END_GET_

  static const luaL_Reg funcs[] = {
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    {"get_value",WRAP(get_value)},
    {"get_list",WRAP(get_list)},
    {"get_map",WRAP(get_map)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    {"type",WRAP(type)},
    {"empty",WRAPMEM(T::empty)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };
}

namespace ProjectionReg{
  typedef Projection T;
  an<T> make(){
    return New<T>();
  }

  string apply(T &t, const string &s){
    string res= s;
    if (t.Apply(&res))
      return res;
    else
      return "";
  }

  static const luaL_Reg funcs[] = {
    {"Projection",WRAP(make)},
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    {"load",WRAPMEM(T::Load)},
    {"apply",WRAP(apply)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
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

  // GetString : overload function
  bool set_string(T &t, const string &path, const string &value) {
    return t.SetString(path, value);
  }

  // GetItem SetItem : overload function
  an<ConfigItem> get_item(T &t, const string & path){
    return t.GetItem(path);
  }

  bool set_item(T &t ,const string &path, an<ConfigItem> item){
    return t.SetItem(path,item);
  }

  bool set_value(T &t, const string &path,  an<ConfigValue>  value) {
    return t.SetItem(path, value);
  }

  bool set_list(T &t, const string &path,  an<ConfigList>  value) {
    return t.SetItem(path, value);
  }

  bool set_map(T &t, const string &path, an<ConfigMap> value) {
    return t.SetItem(path, value);
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

    { "set_bool", WRAPMEM(T::SetBool) },
    { "set_int", WRAPMEM(T::SetInt) },
    { "set_double", WRAPMEM(T::SetDouble) },
    { "set_string", WRAP(set_string) }, // redefine overload function

    { "get_item", WRAP(get_item) }, // redefine overload function
    { "set_item", WRAP(set_item) }, // create new function

    //an<ConfigItem> GetItem(const string& path);
    //an<ConfigValue> GetValue(const string& path);
    //RIME_API an<ConfigList> GetList(const string& path);
    //RIME_API an<ConfigMap> GetMap(const string& path);

    { "get_value", WRAPMEM(T::GetValue) },
    { "get_list", WRAPMEM(T::GetList) },
    { "get_map", WRAPMEM(T::GetMap) },

    { "set_value", WRAP(set_value) }, // create new function
    { "set_list", WRAP(set_list) }, // create new function
    { "set_map", WRAP(set_map)}, // create new function

    { "get_list_size", WRAPMEM(T::GetListSize) },

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
    {"empty",WRAPMEM(T::empty)},
    {"clear",WRAPMEM(T::Clear)},
    {"append_phrase",WRAPMEM(T::AppendPhrase)},
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
  typedef LuaMemory T;


  // XXX: Currently the WRAP macro is not generic enough,
  // so that we need a raw function to get the lua state / parse variable args.
  int raw_make(lua_State *L) {
    // TODO: fix the memory leak
    C_State C;
    int n = lua_gettop(L);
    Lua *lua = Lua::from_state(L);
    if (1 > n)
      return 0;

    Engine *engine= LuaType<Engine *>::todata(L, 1);
    Ticket translatorTicket(engine,"translator");
    translatorTicket.schema = & (LuaType<Schema &>::todata(L, 2) );

    if (3 <= n)
      translatorTicket.name_space = LuaType<string>::todata(L, 3, &C);

    an<T> memoli = New<T>(lua, translatorTicket);
    if (4 <= n && memoli && lua_isfunction(L, 4))
      memoli->memorize( LuaObj::todata(L, 4));

    LuaType<an<T>>::pushdata(L, memoli);
    return 1;
  }

  bool dictLookup(T& memory, const string& input, const bool isExpand,size_t limit) {
    memory.clearDict();
    limit = limit == 0 ? 0xffffffffffffffff : limit;
    if (auto dict = memory.dict())
      return dict->LookupWords(&memory.iter, input, isExpand, limit) > 0;
    else
      return false;
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
    if (auto dict = memory.user_dict())
      return dict->LookupWords(&memory.uter, input, isExpand) > 0;
    else
      return false;
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
    if (auto dict = memory.user_dict())
      return dict->UpdateEntry(entry, commits, new_entry_prefix);
    else
      return false;
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

  static const luaL_Reg funcs[] = {
      {"Memory", raw_make},
      {NULL, NULL},
  };

  std::vector<string> decode(T& memory, Code& code) {
    std::vector<string> res;
    if (auto dict = memory.dict())
      dict->Decode(code,&res);
    return res;
  }
  static const luaL_Reg methods[] = {
      { "dict_lookup", WRAP(dictLookup)},
      { "user_lookup", WRAP(userLookup)},
      { "memorize", WRAPMEM(T::memorize)},
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

  an<T> make(LuaMemory& memory,
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
  double quality(T &t){
    return t.quality();
  }
  void set_quality(T &t, double v){
    t.set_quality(v);
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
    { "quality", WRAP(quality) },
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
    { "quality", WRAP(set_quality) },
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
    { "repr", WRAPMEM(T::repr) },
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

namespace RimeApiReg {
  string get_rime_version() {
    RimeApi* rime = rime_get_api();
    return string(rime->get_version());
  }

  string get_shared_data_dir() {
    RimeApi* rime = rime_get_api();
    return string(rime->get_shared_data_dir());
  }

  string get_user_data_dir() {
    RimeApi* rime = rime_get_api();
    return string(rime->get_user_data_dir());
  }

  string get_sync_dir() {
    RimeApi* rime = rime_get_api();
    return string(rime->get_sync_dir());
  }

  static const luaL_Reg funcs[]= {
    { "get_rime_version", WRAP(get_rime_version) },
    { "get_shared_data_dir", WRAP(get_shared_data_dir) },
    { "get_user_data_dir",  WRAP(get_user_data_dir) },
    { "get_sync_dir",  WRAP(get_sync_dir) },
    { NULL, NULL },
  };

  void init(lua_State *L) {
    lua_createtable(L, 0, 0);
    luaL_setfuncs(L, funcs, 0);
    lua_setglobal(L, "rime_api");
  }
}

namespace SwitcherReg {
  typedef Switcher T;

  an<T> make(Engine *engine) {
    return New<T>(engine);
  }


  static const luaL_Reg funcs[] = {
    { "Switcher", WRAP(make) },
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    { "select_next_schema", WRAPMEM(T::SelectNextSchema) },
    { "is_auto_save", WRAPMEM(T::IsAutoSave) },
    { "refresh_menu", WRAPMEM(T::RefreshMenu) },
    { "activate", WRAPMEM(T::Activate) },
    { "deactivate", WRAPMEM(T::Deactivate) },
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    { "attached_engine", WRAPMEM(T::attached_engine) },
    { "user_config", WRAPMEM(T::user_config) },
    { "active", WRAPMEM(T::active) },
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };
}
namespace OpenccReg{
  typedef opencc::SimpleConverter T;

  an<T> make(const string &filename){
    return New<T>(filename);
  }

  string convert(T &t, const string& text){
    return t.Convert(text);
  }

  static const luaL_Reg funcs[] = {
    { "Opencc", WRAP(make) },
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    { "convert", WRAP(convert) },
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };
}

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
  EXPORT(ConfigItemReg, L);
  EXPORT(ConfigValueReg, L);
  EXPORT(ConfigListReg, L);
  EXPORT(ConfigMapReg, L);
  EXPORT(ProjectionReg, L);
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
  EXPORT(SwitcherReg, L);
  EXPORT(OpenccReg, L);
  LogReg::init(L);
  RimeApiReg::init(L);

  export_type(L, LuaType<the<SchemaReg::T>>::name(), LuaType<the<SchemaReg::T>>::gc,
              SchemaReg::funcs, SchemaReg::methods, SchemaReg::vars_get, SchemaReg::vars_set);
}
