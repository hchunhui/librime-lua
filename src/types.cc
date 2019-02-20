#include <rime/candidate.h>
#include <rime/translation.h>
#include <rime/segmentation.h>
#include <rime/gear/translator_commons.h>
#include <rime/dict/reverse_lookup_dictionary.h>
#include "lua_templates.h"

namespace rime {

//--- wrappers for Segment &
namespace SegmentReg {
  typedef const Segment &T;

  static const luaL_Reg funcs[] = {
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    { "start", WRAPMEM_GET(T, Segment::start) },
    { "_end", WRAPMEM_GET(T, Segment::end) }, // end is keyword in Lua...
    { "length", WRAPMEM_GET(T, Segment::length) },
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };
}

//--- wrappers for an<Candidate>
namespace CandidateReg {
  typedef an<Candidate> T;

  static string dynamic_type(T c) {
    if (Is<Phrase>(c))
      return "Phrase";
    if (Is<SimpleCandidate>(c))
      return "Simple";
    if (Is<ShadowCandidate>(c))
      return "Shadow";
    if (Is<UniquifiedCandidate>(c))
      return "uniquified";
    return "Other";
  }

  static void set_text(T c, const string &v) {
    if (Is<SimpleCandidate>(c))
      As<SimpleCandidate>(c)->set_text(v);
  }

  static void set_comment(T c, const string &v) {
    if (Is<Phrase>(c))
      As<Phrase>(c)->set_comment(v);
    else if (Is<SimpleCandidate>(c))
      As<SimpleCandidate>(c)->set_comment(v);
  }

  static void set_preedit(T c, const string &v) {
    if (Is<Phrase>(c))
      As<Phrase>(c)->set_preedit(v);
    else if (Is<SimpleCandidate>(c))
      As<SimpleCandidate>(c)->set_preedit(v);
  }

  static T make(const string type,
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
    { "get_genuine", WRAPT(Candidate::GetGenuineCandidate, T, T) },
    { "get_genuines", WRAPT(Candidate::GetGenuineCandidates, vector<T>, T) },
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    { "type", WRAPMEM(T, Candidate::type) },
    { "start", WRAPMEM(T, Candidate::start) },
    { "_end", WRAPMEM(T, Candidate::end) }, // end is keyword in Lua...
    { "quality", WRAPMEM(T, Candidate::quality) },
    { "text", WRAPMEM(T, Candidate::text) },
    { "comment", WRAPMEM(T, Candidate::comment) },
    { "preedit", WRAPMEM(T, Candidate::preedit) },
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { "type", WRAPMEM(T, Candidate::set_type) },
    { "start", WRAPMEM(T, Candidate::set_start) },
    { "_end", WRAPMEM(T, Candidate::set_end) },
    { "quality", WRAPMEM(T, Candidate::set_quality) },
    { "text", WRAP(set_text) },
    { "comment", WRAP(set_comment) },
    { "preedit", WRAP(set_preedit) },
    { NULL, NULL },
  };
}

//--- wrappers for an<Translation>
an<Translation> lua_translation_new(Lua *lua, int id);
namespace TranslationReg {
  typedef an<Translation> T;

  int raw_make(lua_State *L) {
    Lua *lua = Lua::from_state(L);
    int n = lua_gettop(L);

    if (n < 1)
      return 0;

    int id = lua->newthread(L, n - 1);
    auto r = lua_translation_new(lua, id);
    LuaType<an<Translation>>::pushdata(L, r);
    return 1;
  }

  static an<Candidate> next(T t) {
    auto c = t->Peek();
    t->Next();
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
  typedef an<ReverseDb> T;

  static T make(const string &file) {
    T db = New<ReverseDb>(string(RimeGetUserDataDir()) +  "/" + file);
    db->Load();
    return db;
  }

  static string lookup(T db, const string &key) {
    string res;
    if (db->Lookup(key, &res))
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
    export_type(L, LuaType<ns::T>::name().c_str(), LuaType<ns::T>::gc, \
                ns::funcs, ns::methods, ns::vars_get, ns::vars_set)
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
