#include <rime/candidate.h>
#include <rime/translation.h>
#include <rime/segmentation.h>
#include <rime/gear/translator_commons.h>
#include <rime/dict/reverse_lookup_dictionary.h>
#include "lua_templates.h"

namespace rime {

//--- wrappers for Segment
namespace SegmentReg {
  typedef Segment T;

  static T make(int start_pos, int end_pos) {
    return Segment(start_pos, end_pos);
  }

  static string get_status(const T &t) {
    switch (t.status) {
    case T::kVoid: return "kVoid";
    case T::kGuess: return "kGuess";
    case T::kSelected: return "kSelected";
    case T::kConfirmed: return "kConfirmed";
    }
    return "";
  }

  static void set_status(T &t, const string &r) {
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

  static string dynamic_type(T &c) {
    if (dynamic_cast<Phrase *>(&c))
      return "Phrase";
    if (dynamic_cast<SimpleCandidate *>(&c))
      return "Simple";
    if (dynamic_cast<ShadowCandidate *>(&c))
      return "Shadow";
    if (dynamic_cast<UniquifiedCandidate *>(&c))
      return "uniquified";
    return "Other";
  }

  static void set_text(T &c, const string &v) {
    if (auto p = dynamic_cast<SimpleCandidate *>(&c))
      p->set_text(v);
  }

  static void set_comment(T &c, const string &v) {
    if (auto p = dynamic_cast<Phrase *>(&c))
      p->set_comment(v);
    else if (auto p = dynamic_cast<SimpleCandidate *>(&c))
      p->set_comment(v);
  }

  static void set_preedit(T &c, const string &v) {
    if (auto p = dynamic_cast<Phrase *>(&c))
      p->set_preedit(v);
    else if (auto p = dynamic_cast<SimpleCandidate *>(&c))
      p->set_preedit(v);
  }

  static an<T> make(const string type,
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
an<Translation> lua_translation_new(Lua *lua, an<LuaObj> o);
namespace TranslationReg {
  typedef Translation T;

  int raw_make(lua_State *L) {
    Lua *lua = Lua::from_state(L);
    int n = lua_gettop(L);

    if (n < 1)
      return 0;

    auto o = lua->newthread(L, n - 1);
    auto r = lua_translation_new(lua, o);
    LuaType<an<Translation>>::pushdata(L, r);
    return 1;
  }

  static optional<an<Candidate>> next(T &t) {
    if (t.exhausted())
      return {};

    auto c = t.Peek();
    t.Next();
    return c;
  }

  static int raw_iter(lua_State *L) {
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

  static an<T> make(const string &file) {
    an<T> db = New<ReverseDb>(string(RimeGetUserDataDir()) +  "/" + file);
    db->Load();
    return db;
  }

  static string lookup(T &db, const string &key) {
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

//--- Lua
#define EXPORT(ns, L) \
  do { \
  export_type(L, LuaType<ns::T>::name().c_str(), LuaType<ns::T>::gc,     \
              ns::funcs, ns::methods, ns::vars_get, ns::vars_set);       \
  export_type(L, LuaType<ns::T &>::name().c_str(), NULL, \
              ns::funcs, ns::methods, ns::vars_get, ns::vars_set); \
  export_type(L, LuaType<const ns::T>::name().c_str(), LuaType<ns::T>::gc,     \
              ns::funcs, ns::methods, ns::vars_get, ns::vars_set);       \
  export_type(L, LuaType<const ns::T &>::name().c_str(), NULL, \
              ns::funcs, ns::methods, ns::vars_get, ns::vars_set); \
  export_type(L, LuaType<an<ns::T>>::name().c_str(), NULL,         \
              ns::funcs, ns::methods, ns::vars_get, ns::vars_set); \
  export_type(L, LuaType<an<const ns::T>>::name().c_str(), NULL,   \
              ns::funcs, ns::methods, ns::vars_get, ns::vars_set); \
  export_type(L, LuaType<ns::T *>::name().c_str(), NULL,           \
              ns::funcs, ns::methods, ns::vars_get, ns::vars_set); \
  export_type(L, LuaType<const ns::T *>::name().c_str(), NULL,     \
              ns::funcs, ns::methods, ns::vars_get, ns::vars_set); \
  } while (0)

namespace LuaImpl {
  void export_type(lua_State *L,
                   const char *name, lua_CFunction gc,
                   const luaL_Reg *funcs, const luaL_Reg *methods,
                   const luaL_Reg *vars_get, const luaL_Reg *vars_set);

  void types_init(lua_State *L) {
    EXPORT(SegmentReg, L);
    EXPORT(CandidateReg, L);
    EXPORT(TranslationReg, L);
    EXPORT(ReverseDbReg, L);
  }
}

}  // namespace rime
