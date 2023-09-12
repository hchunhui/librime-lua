#include <rime/translation.h>
#include <rime/candidate.h>
#include "lib/lua_export_type.h"
#include "lib/luatype_boost_optional.h"

namespace {
using namespace rime;

template <typename T>
static an<Translation> to_translation(an<T> &t) {
  return t;
};

template <typename T>
optional<an<Candidate>> next(an<T> &t) {
  if (t->exhausted())
    return {};
  auto c = t->Peek();
  t->Next();
  return c;
};

template<typename T>
void append(an<T> t, an<Candidate> o){
  t->Append(o);
};

template<typename T>
void append(an<T> t, an<Translation> o){
  *t += o;
};

template <typename T>
static int raw_iter(lua_State *L) {
  lua_pushcfunction(L, WRAP(next<T>));
  lua_pushvalue(L, 1);
  return 2;
};

template <typename T, typename I>
int raw_append(lua_State* L) {
  int n = lua_gettop(L);
  if ( 1 >= n)
    return n;

  an<T> r = LuaType<an<T>>::todata(L,1);
  for(int i = 2; i <=n; i++) {
    an<I> o = LuaType<an<I>>::todata(L, i);
    append<T>(r,o);
  }
  lua_pop(L, n-1);
  return 1;
};

template <typename T, typename I>
int raw_make(lua_State* L) {
  static const CandidateList cl;
  an<T> t = New<T>();
  LuaType<an<T>>::pushdata(L, t);
  if (lua_gettop(L) > 1) {
    lua_insert(L, 1);
    return raw_append<T,I>(L);
  }
  return 1;
};


namespace MergedTranslationReg {
  typedef MergedTranslation T;
  typedef Translation IT;

  int raw_make(lua_State* L) {
    static const CandidateList cl;
    an<T> t = New<T>(cl);
    LuaType<an<T>>::pushdata(L, t);
    if (lua_gettop(L) > 1) {
      lua_insert(L, 1);
      return raw_append<T,IT>(L);
    }
    return 1;
  }

  static const luaL_Reg funcs[] = {
    { "MergedTranslation", raw_make},
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    { "iter", raw_iter<T>},
    { "append", raw_append<T,IT>},
    { "_peek", WRAPMEM(T, Peek)},
    { "_next", WRAPMEM(T, Next)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    { "exhausted", WRAPMEM(T, exhausted)},
    { "translation", WRAP(to_translation<T>)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };
}

namespace UnionTranslationReg {
  typedef UnionTranslation T;
  typedef Translation IT;

  static const luaL_Reg funcs[] = {
    { "UnionTranslation", raw_make<T,IT>},
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    { "iter", raw_iter<T>},
    { "append", raw_append<T,IT>},
    { "_peek", WRAPMEM(T, Peek)},
    { "_next", WRAPMEM(T, Next)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    { "exhausted", WRAPMEM(T, exhausted)},
    { "translation", WRAP(to_translation<T>)},
    { NULL, NULL},
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };
}

namespace FifoTranslationReg {
  typedef FifoTranslation T;
  typedef Candidate IT;

  static const luaL_Reg funcs[] = {
    { "FifoTranslation", raw_make<T,IT>},
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    { "iter", raw_iter<T>},
    { "append", raw_append<T,IT>},
    { "_peek", WRAPMEM(T, Peek)},
    { "_next", WRAPMEM(T, Next)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    { "exhausted", WRAPMEM(T, exhausted)},
    { "translation", WRAP(to_translation<T>)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };
}
} // namespace

void LUAWRAPPER_LOCAL translation_init(lua_State *L) {
  EXPORT(UnionTranslationReg, L);
  EXPORT(FifoTranslationReg, L);
  EXPORT(MergedTranslationReg, L);
}
