#ifndef LIB_LUA_TEMPLATES_H_
#define LIB_LUA_TEMPLATES_H_

#include "lua.h"
#include <boost/type_index.hpp>
#include <typeinfo>
#include <vector>
#include <set>

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#if LUA_VERSION_NUM >= 502
#define xlua_resume(L, a) lua_resume(L, NULL, a)
#define LUA_UNPACK "table.unpack"
#else
#define xlua_resume lua_resume
#define lua_rawlen lua_objlen
#define LUA_UNPACK "unpack"
#endif

//--- LuaType
// Generic case (includes pointers)
template<typename T>
struct LuaType {
  static const std::string name() {
    return boost::typeindex::type_id_with_cvr<T>().pretty_name();
    // return string("V") + typeid(T).name();
  }

  static int gc(lua_State *L) {
    T *o = (T *) luaL_checkudata(L, 1, name().c_str());
    o->~T();
    return 0;
  }

  template<typename U>
  struct X {
    static bool pushnil(lua_State *L, U &o) {
      return false;
    }
  };

  template<typename U>
  struct X<U *> {
    static bool pushnil(lua_State *L, U *o) {
      if (o)
        return false;
      lua_pushnil(L);
      return true;
    }
  };

  template<typename U>
  struct X<std::shared_ptr<U>> {
    static bool pushnil(lua_State *L, std::shared_ptr<U> &o) {
      if (o)
        return false;
      lua_pushnil(L);
      return true;
    }
  };

  static void pushdata(lua_State *L, T &o) {
    if (X<T>::pushnil(L, o))
      return;

    void *u = lua_newuserdata(L, sizeof(T));
    new(u) T(o);
    luaL_getmetatable(L, name().c_str());
    if (lua_isnil(L, -1)) {
      // If T is not registered,
      // registers a "__gc" to prevent memory leaks.
      lua_pop(L, 1);
      luaL_newmetatable(L, name().c_str());
      lua_pushstring(L, "__gc");
      lua_pushcfunction(L, gc);
      lua_settable(L, -3);
    }
    lua_setmetatable(L, -2);
  }

  static T &todata(lua_State *L, int i) {
    typedef typename std::remove_const<T>::type U;
    T *o = (T *) luaL_testudata(L, i, name().c_str());
    if (o)
      return *o;

    if (std::is_const<T>::value) {
      T *o = (T *) luaL_testudata(L, i, LuaType<U>::name().c_str());
      if (o)
        return *o;
    }

    const char *msg = lua_pushfstring(L, "%s expected", name().c_str());
    luaL_argerror(L, i, msg);
    return *((T *) NULL);
  }
};

// References
template<typename T>
struct LuaType<T &> {
  static const std::string name() {
    return boost::typeindex::type_id_with_cvr<T &>().pretty_name();
    // return string("R") + typeid(T).name();
  }

  static void pushdata(lua_State *L, T &o) {
    T **u = (T**) lua_newuserdata(L, sizeof(T *));
    *u = std::addressof(o);
    luaL_setmetatable(L, name().c_str());
  }

  static T &todata(lua_State *L, int i) {
    typedef typename std::remove_const<T>::type U;
    T **po = (T **) luaL_testudata(L, i, name().c_str());
    if (po)
      return **po;

    if (std::is_const<T>::value) {
      T **po = (T **) luaL_testudata(L, i, LuaType<U &>::name().c_str());
      if (po)
        return **po;
    }

    auto ao = (std::shared_ptr<T> *) luaL_testudata(L, i, LuaType<std::shared_ptr<T>>::name().c_str());
    if (ao)
      return *(*ao).get();

    if (std::is_const<T>::value) {
      auto ao = (std::shared_ptr<T> *) luaL_testudata(L, i, LuaType<std::shared_ptr<U>>::name().c_str());
      if (ao)
        return *(*ao).get();
    }

    T **p = (T **) luaL_testudata(L, i, LuaType<T *>::name().c_str());
    if (p)
      return **p;

    if (std::is_const<T>::value) {
      T **p = (T **) luaL_testudata(L, i, LuaType<U *>::name().c_str());
      if (p)
        return **p;
    }

    T *o = (T *) luaL_testudata(L, i, LuaType<T>::name().c_str());
    if (o)
      return *o;

    if (std::is_const<T>::value) {
      T *o = (T *) luaL_testudata(L, i, LuaType<U>::name().c_str());
      if (o)
        return *o;
    }

    const char *msg = lua_pushfstring(L, "%s expected", name().c_str());
    luaL_argerror(L, i, msg);
    return *((T *) NULL);
  }
};

// Generic Lua Object
template<>
struct LuaType<std::shared_ptr<LuaObj>> {
  static void pushdata(lua_State *L, std::shared_ptr<LuaObj> &o) {
    LuaObj::pushdata(L, o);
  }

  static std::shared_ptr<LuaObj> todata(lua_State *L, int i) {
    return LuaObj::todata(L, i);
  }
};

// C Function
template<>
struct LuaType<lua_CFunction> {
  static void pushdata(lua_State *L, lua_CFunction f) {
    lua_pushcfunction(L, f);
  }
};

// Optional
template<typename T>
struct LuaType<boost::optional<T>> {
  static void pushdata(lua_State *L, boost::optional<T> o) {
    if (o)
      LuaType<T>::pushdata(L, *o);
    else
      lua_pushnil(L);
  }

  static boost::optional<T> todata(lua_State *L, int i) {
    if (lua_type(L, i) == LUA_TNIL)
      return {};
    else
      return LuaType<T>::todata(L, i);
  }
};

// Bool
template<>
struct LuaType<bool> {
  static void pushdata(lua_State *L, bool o) {
    lua_pushboolean(L, o);
  }

  static bool todata(lua_State *L, int i) {
    return lua_toboolean(L, i);
  }
};

// Integers
template<>
struct LuaType<int> {
  static void pushdata(lua_State *L, lua_Integer o) {
    lua_pushinteger(L, o);
  }

  static lua_Integer todata(lua_State *L, int i) {
    return luaL_checkinteger(L, i);
  }
};

template<>
struct LuaType<unsigned int> : LuaType<int> {};

template<>
struct LuaType<long> : LuaType<int> {};

template<>
struct LuaType<unsigned long> : LuaType<int> {};

template<>
struct LuaType<long long> : LuaType<int> {};

template<>
struct LuaType<unsigned long long> : LuaType<int> {};

template<>
struct LuaType<short> : LuaType<int> {};

template<>
struct LuaType<unsigned short> : LuaType<int> {};

template<>
struct LuaType<char> : LuaType<int> {};

template<>
struct LuaType<unsigned char> : LuaType<int> {};

// Floats
template<>
struct LuaType<double> {
  static void pushdata(lua_State *L, lua_Number o) {
    lua_pushnumber(L, o);
  }

  static lua_Number todata(lua_State *L, int i) {
    return luaL_checknumber(L, i);
  }
};

template<>
struct LuaType<float> : LuaType<double> {};

template<>
struct LuaType<long double> : LuaType<double> {};

// Strings
template<>
struct LuaType<std::string> {
  static void pushdata(lua_State *L, const std::string &o) {
    lua_pushstring(L, o.c_str());
  }

  static std::string todata(lua_State *L, int i) {
    return std::string(luaL_checkstring(L, i));
  }
};

template<>
struct LuaType<const std::string> : LuaType<std::string> {};

template<>
struct LuaType<const std::string &> : LuaType<std::string> {};

/* string references/pointers are not supported now */

// Arrays
// The index starts form 1 in Lua...
template<typename T>
struct LuaType<std::vector<T>> {
  static void pushdata(lua_State *L, std::vector<T> &o) {
    int n = o.size();
    lua_createtable(L, n, 0);
    for (int i = 0; i < n; i++) {
      LuaType<T>::pushdata(L, o[i]);
      lua_rawseti(L, -2, i + 1);
    }
  }
  static std::vector<T> todata(lua_State *L, int j) {
    std::vector<T> o;
    int n = lua_rawlen(L, j);
    for (int i = 0; i < n; i++) {
      lua_rawgeti(L, j, i + 1);
      o.push_back(LuaType<T>::todata(L, -1));
      lua_pop(L, 1);
    }
    return o;
  }
};

// Sets
template<typename T>
struct LuaType<std::set<T>> {
  static void pushdata(lua_State *L, const std::set<T> &o) {
    lua_createtable(L, 0, o.size());
    for (auto it = o.begin(); it != o.end(); it++) {
      LuaType<T>::pushdata(L, *it);
      LuaType<bool>::pushdata(L, true);
      lua_rawset(L, -3);
    }
    luaL_setmetatable(L, "__set");
  }
  static std::set<T> todata(lua_State *L, int j) {
    std::set<T> o;
    o.clear();
    lua_pushnil(L);  /* first key */
    while (lua_next(L, j) != 0) {
      o.insert(LuaType<T>::todata(L, -2));
      lua_pop(L, 1);
    }
    return o;
  }
};

template<typename T>
struct LuaType<const std::vector<T>> : LuaType<std::vector<T>> {};

template<typename T>
struct LuaType<const std::vector<T> &> : LuaType<std::vector<T>> {};

// Helper function for push multiple data
static void pushdataX(lua_State *L) {}

template<typename T>
static void pushdataX(lua_State *L, T &o) {
  LuaType<T>::pushdata(L, o);
}

template<typename T, typename ... Targs>
static void pushdataX(lua_State *L, T &o, Targs ... oargs) {
  LuaType<T>::pushdata(L, o);
  pushdataX<Targs ...>(L, oargs ...);
}

// --- Lua call/resume
template <typename O>
O Lua::getglobal(const std::string &v) {
  lua_getglobal(L_, v.c_str());
  O o = LuaType<O>::todata(L_, -1);
  lua_pop(L_, 1);
  return o;
}

template <typename ... I>
std::shared_ptr<LuaObj> Lua::newthread(I ... input) {
  pushdataX<I ...>(L_, input ...);
  return newthreadx(L_, sizeof...(input));
}

template <typename O>
boost::optional<O> Lua::resume(std::shared_ptr<LuaObj> f) {
  LuaObj::pushdata(L_, f);
  lua_State *C = lua_tothread(L_, -1);
  lua_pop(L_, 1);

  int status = xlua_resume(C, 0);
  if (status == LUA_YIELD) {
    O c = LuaType<O>::todata(C, -1);
    lua_pop(C, 1);
    return c;
  } else {
    if (status != LUA_OK) {
      const char *e = lua_tostring(C, -1);
      printf("resume(err=%d): %s\n", status, e);
    }
    return {};
  }
}

template <typename O, typename ... I>
O Lua::call(I ... input) {
  pushdataX<I ...>(L_, input ...);

  int status = lua_pcall(L_, sizeof...(input) - 1, 1, 0);
  if (status != LUA_OK) {
    const char *e = lua_tostring(L_, -1);
    printf("call(err=%d): %s\n", status, e);
    lua_pop(L_, 1);
    return O();
  }

  O r = LuaType<O>::todata(L_, -1);
  lua_pop(L_, 1);
  return r;
}

// --- LuaWrapper
// WRAP(f): wraps function f
// WRAPMEM(C::f): wraps member function C::f
// WRAPMEM_GET/SET(C::f): wraps member variable C::f

template<typename F, F f>
struct LuaWrapper;

// LuaWrapper: R(*)(T...) -> int (*)(lua_State *)
template<typename S, typename... T, S(*f)(T...)>
struct LuaWrapper<S(*)(T...), f> {
  // args = R
  template<typename R, typename... A>
  struct args {
    template<typename... Us>
    struct aux {
      template<int n, typename Is_void>
      struct ret {
        static int wrap(lua_State *L, Us... us) {
          R r = f(us...);
          LuaType<R>::pushdata(L, r);
          return 1;
        }
      };

      template<int n>
      struct ret<n, void> {
        static int wrap(lua_State *L, Us... us) {
          f(us...);
          return 0;
        }
      };

      template<int n>
      static int wrap(lua_State *L, Us... us) {
        return ret<n, R>::wrap(L, us...);
      }
    };
  };

  // args = R :: A :: As
  template<typename R, typename A, typename... As>
  struct args<R, A, As...> {
    template<typename... Us>
    struct aux {
      template <int n>
      static int wrap(lua_State *L, Us... us) {
        A t = LuaType<A>::todata(L, n);
        return args<R, As...>::
          template aux<Us..., A &>::
          template wrap<n + 1>(L, us..., t);
      }
    };
  };

  static int wrap(lua_State *L) {
    return args<S, T...>::
      template aux<>::
      template wrap<1>(L);
  }
};

// MemberWrapper: R (C::*)(T..) -> R (C &, T...)
// MemberWrapper(get variable): R (C::*) -> R (C &)
// MemberWrapper(set variable): R (C::*) -> void (C &, R)
template<typename F, F f>
struct MemberWrapper;

template<typename R, typename C, typename... T, R (C::*f)(T...)>
struct MemberWrapper<R (C::*)(T...), f> {
  static R wrap(C &c, T... t) {
    return (c.*f)(t...);
  }
};

template<typename R, typename C, typename... T, R (C::*f)(T...) const>
struct MemberWrapper<R (C::*)(T...) const, f> {
  static R wrap(const C &c, T... t) {
    return (c.*f)(t...);
  }
};

template<typename R, typename C, R C::*f>
struct MemberWrapper<R (C::*), f> {
  static R wrap_get(const C &c) {
    return c.*f;
  }

  static void wrap_set(C &c, R r) {
    c.*f = r;
  }
};

#define WRAP(f) (&(LuaWrapper<decltype(&f), &f>::wrap))
#define WRAPMEM(f) (&(LuaWrapper<decltype(&MemberWrapper<decltype(&f), &f>::wrap), \
                                          &MemberWrapper<decltype(&f), &f>::wrap>::wrap))
#define WRAPMEM_GET(f) (&(LuaWrapper<decltype(&MemberWrapper<decltype(&f), &f>::wrap_get), \
                                              &MemberWrapper<decltype(&f), &f>::wrap_get>::wrap))
#define WRAPMEM_SET(f) (&(LuaWrapper<decltype(&MemberWrapper<decltype(&f), &f>::wrap_set), \
                                              &MemberWrapper<decltype(&f), &f>::wrap_set>::wrap))

#endif /* LIB_LUA_TEMPLATES_H_ */
