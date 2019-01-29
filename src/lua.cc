#include <rime/candidate.h>
#include <rime/translation.h>
#include <rime/segmentation.h>
#include <rime/gear/translator_commons.h>
#include <rime/dict/reverse_lookup_dictionary.h>
#include "lua.h"
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
int raw_lua_translation(lua_State *L);
namespace TranslationReg {
  typedef an<Translation> T;

  static int raw_next(lua_State *L) {
    an<Translation> t = LuaType<an<Translation>>::todata(L, 1);
    auto c = t->Peek();
    if (!c) {
      lua_pushnil(L);
    } else {
      LuaType<an<Candidate>>::pushdata(L, c);
      t->Next();
    }
    return 1;
  }

  static int raw_iter(lua_State *L) {
    lua_pushcfunction(L, raw_next);
    lua_pushvalue(L, 1);
    return 2;
  }

  static const luaL_Reg funcs[] = {
    { "Translation", raw_lua_translation },
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
namespace LuaImpl {
  static int index(lua_State *L) {
    if (luaL_getmetafield(L, 1, "methods") != LUA_TNIL) {
      lua_pushvalue(L, 2);
      lua_rawget(L, -2);
      if (!lua_isnil(L, -1)) {
        return 1;
      } else {
        lua_pop(L, 1);
      }
    }

    if (luaL_getmetafield(L, 1, "vars_get") != LUA_TNIL) {
      lua_pushvalue(L, 2);
      lua_rawget(L, -2);
      if (!lua_isnil(L, -1)) {
        auto f = lua_tocfunction(L, -1);
        lua_pop(L, 1);
        if (f) {
          lua_remove(L, 2);
          return f(L);
        }
      }
    }
    return 0;
  }

  static int newindex(lua_State *L) {
    if (luaL_getmetafield(L, 1, "vars_set") != LUA_TNIL) {
      lua_pushvalue(L, 2);
      lua_rawget(L, -2);
      if (!lua_isnil(L, -1)) {
        auto f = lua_tocfunction(L, -1);
        lua_pop(L, 1);
        if (f) {
          lua_remove(L, 2);
          return f(L);
        }
      }
    }
    return 0;
  }

  template<typename T>
  static void export_type(lua_State *L,
                          const luaL_Reg *funcs, const luaL_Reg *methods,
                          const luaL_Reg *vars_get, const luaL_Reg *vars_set) {
    for (int i = 0; funcs[i].name; i++) {
      lua_register(L, funcs[i].name, funcs[i].func);
    }

    luaL_newmetatable(L, LuaType<T>::name().c_str());
    lua_pushstring(L, "__gc");
    lua_pushcfunction(L, LuaType<T>::gc);
    lua_settable(L, -3);
    lua_pushstring(L, "methods");
    lua_createtable(L, 0, 4);
    luaL_setfuncs(L, methods, 0);
    lua_settable(L, -3);
    lua_pushstring(L, "vars_get");
    lua_createtable(L, 0, 4);
    luaL_setfuncs(L, vars_get, 0);
    lua_settable(L, -3);
    lua_pushstring(L, "vars_set");
    lua_createtable(L, 0, 4);
    luaL_setfuncs(L, vars_set, 0);
    lua_settable(L, -3);
    lua_pushstring(L, "__index");
    lua_pushcfunction(L, index);
    lua_settable(L, -3);
    lua_pushstring(L, "__newindex");
    lua_pushcfunction(L, newindex);
    lua_settable(L, -3);
  }

  static int yield(lua_State *L) {
    return lua_yield(L, lua_gettop(L));
  }

  extern "C" void xluaopen_utf8(lua_State *);

  static const char makeclosurekey = 'k';

  static int pmain(lua_State *L) {
    luaL_openlibs(L);
    xluaopen_utf8(L);
    lua_register(L, "yield", yield);

    export_type<SegmentReg::T>(L, SegmentReg::funcs, SegmentReg::methods,
                               SegmentReg::vars_get, SegmentReg::vars_set);
    export_type<CandidateReg::T>(L, CandidateReg::funcs, CandidateReg::methods,
                                 CandidateReg::vars_get, CandidateReg::vars_set);
    export_type<TranslationReg::T>(L, TranslationReg::funcs, TranslationReg::methods,
                                   TranslationReg::vars_get, TranslationReg::vars_set);
    export_type<ReverseDbReg::T>(L, ReverseDbReg::funcs, ReverseDbReg::methods,
                                 ReverseDbReg::vars_get, ReverseDbReg::vars_set);

    int status = luaL_dofile(L, (string(RimeGetUserDataDir()) + "/rime.lua").c_str());
    if (status != LUA_OK) {
      const char *e = lua_tostring(L, -1);
      printf("dofile(err=%d): %s\n", status, e);
    }

    lua_pushlightuserdata(L, (void *)&makeclosurekey);
    luaL_dostring(L, "return function (f, ...)\n"
                  "local args = {...}\n"
                  "return (function () return f(" LUA_UNPACK "(args)) end)\n"
                  "end\n");
    lua_settable(L, LUA_REGISTRYINDEX);

    return 0;
  }

  static const char luakey = 'k';
}

Lua::Lua() {
  L_ = luaL_newstate();
  if (L_) {
    lua_pushlightuserdata(L_, (void *)&LuaImpl::luakey);
    lua_pushlightuserdata(L_, (void *)this);
    lua_settable(L_, LUA_REGISTRYINDEX);

    lua_pushcfunction(L_, &LuaImpl::pmain);
    int status = lua_pcall(L_, 0, 1, 0);
    if (status != LUA_OK) {
      const char *e = lua_tostring(L_, -1);
      printf("lua init(err=%d): %s\n", status, e);
    }
  }
}

Lua::~Lua() {
  printf("lua_exit\n");
  lua_close(L_);
}

Lua *Lua::from_state(lua_State *L) {
  Lua *lua;
  lua_pushlightuserdata(L, (void *)&LuaImpl::luakey);
  lua_gettable(L, LUA_REGISTRYINDEX);
  lua = (Lua *) lua_touserdata(L, -1);
  lua_pop(L, 1);
  return lua;
}

int Lua::newthread(lua_State *L, int nargs) {
  lua_State *C = lua_newthread(L_);
  int id = luaL_ref(L_, LUA_REGISTRYINDEX);

  lua_pushlightuserdata(C, (void *)&LuaImpl::makeclosurekey);
  lua_gettable(C, LUA_REGISTRYINDEX);
  lua_xmove(L, C, 1 + nargs);
  lua_call(C, 1 + nargs, 1);

  return id;
}

}  // namespace rime
