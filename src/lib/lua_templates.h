#ifndef LIB_LUA_TEMPLATES_H_
#define LIB_LUA_TEMPLATES_H_

#include "lua.h"
#include <typeinfo>
#include <vector>
#include <set>
#include <cstring>

#ifdef __GNUC__
#define LUAWRAPPER_LOCAL __attribute__((visibility("hidden")))
#else
#define LUAWRAPPER_LOCAL
#endif

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include "lua-compat.h"
}

//--- C_State
// Lua uses setjmp/longjmp to handle Lua exceptions by default,
// unfortunately the approach may cause memory leakage in C++. It's
// possible to let Lua use C++ exceptions, but little of freestanding
// Lua libraries are compiled with C++.
//
// To workaround it, the code sequence protected by lua_pcall() should
// not rely on destructors. Instead the resources should be registered
// here, so that they can be freed outside the call when exception
// happens.
class LUAWRAPPER_LOCAL C_State {
  struct B {
    virtual ~B() {};
  };

  template<typename T>
  struct I : public B {
    T value;
    template<typename... Args>
    I(Args &&... args)
      : value(std::forward<Args>(args)...) {}
  };

  std::vector<std::unique_ptr<B>> list;
public:
  template<typename T, typename... Args>
  T &alloc(Args &&... args) {
    auto r = new I<T>(std::forward<Args>(args)...);
    list.emplace_back(r);
    return r->value;
  }
};

struct LUAWRAPPER_LOCAL LuaTypeInfo {
  const std::type_info *ti;
  size_t hash;

  template<typename T>
  static const LuaTypeInfo &make() {
    auto &i = typeid(T);
    static LuaTypeInfo r = {&i, i.hash_code()};
    return r;
  }

  const char *name() const {
    return ti->name();
  }

  bool operator==(const LuaTypeInfo &o) const {
    return hash == o.hash && *ti == *o.ti;
  }
};

//--- LuaType
// Generic case (includes pointers)
template<typename T>
struct LUAWRAPPER_LOCAL LuaType {
  static const LuaTypeInfo *type() {
    return &LuaTypeInfo::make<LuaType<T>>();
  }

  static int gc(lua_State *L) {
    T *o = (T *) luaL_checkudata(L, 1, type()->name());
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
    luaL_getmetatable(L, type()->name());
    if (lua_isnil(L, -1)) {
      // If T is not registered,
      // registers a "__gc" to prevent memory leaks.
      lua_pop(L, 1);
      luaL_newmetatable(L, type()->name());
      lua_pushlightuserdata(L, (void *) type());
      lua_setfield(L, -2, "type");
      lua_pushcfunction(L, gc);
      lua_setfield(L, -2, "__gc");
    }
    lua_setmetatable(L, -2);
  }

  static T &todata(lua_State *L, int i, C_State * = NULL) {
    typedef typename std::remove_const<T>::type U;

    if (lua_getmetatable(L, i)) {
      lua_getfield(L, -1, "type");
      auto ttype = (const LuaTypeInfo *) lua_touserdata(L, -1);
      if (ttype) {
        void *_p = lua_touserdata(L, i);
        if (*ttype == *type() ||
            *ttype == *LuaType<U>::type()) {
          auto o = (T *) _p;
          lua_pop(L, 2);
          return *o;
        }
      }
      lua_pop(L, 2);
    }

    const char *msg = lua_pushfstring(L, "%s expected", type()->name());
    luaL_argerror(L, i, msg);
    abort(); // unreachable
  }
};

// References
template<typename T>
struct LuaType<T &> {
  static const LuaTypeInfo *type() {
    return &LuaTypeInfo::make<LuaType<T &>>();
  }

  static void pushdata(lua_State *L, T &o) {
    T **u = (T**) lua_newuserdata(L, sizeof(T *));
    *u = std::addressof(o);
    luaL_setmetatable(L, type()->name());
  }

  static T &todata(lua_State *L, int i, C_State * = NULL) {
    typedef typename std::remove_const<T>::type U;

    if (lua_getmetatable(L, i)) {
      lua_getfield(L, -1, "type");
      auto ttype = (const LuaTypeInfo *) lua_touserdata(L, -1);
      if (ttype) {
        void *_p = lua_touserdata(L, i);
        if (*ttype == *type() ||
            *ttype == *LuaType<U &>::type()) {
          auto po = (T **) _p;
          lua_pop(L, 2);
          return **po;
        }

        if (*ttype == *LuaType<std::shared_ptr<T>>::type() ||
            *ttype == *LuaType<std::shared_ptr<U>>::type()) {
          auto ao = (std::shared_ptr<T> *) _p;
          lua_pop(L, 2);
          return *(*ao).get();
        }

        if (*ttype == *LuaType<std::unique_ptr<T>>::type() ||
            *ttype == *LuaType<std::unique_ptr<U>>::type()) {
          auto ao = (std::unique_ptr<T> *) _p;
          lua_pop(L, 2);
          return *(*ao).get();
        }

        if (*ttype == *LuaType<T *>::type() ||
            *ttype == *LuaType<U *>::type()) {
          auto p = (T **) _p;
          lua_pop(L, 2);
          return **p;
        }

        if (*ttype == *LuaType<T>::type() ||
            *ttype == *LuaType<U>::type()) {
          auto o = (T *) _p;
          lua_pop(L, 2);
          return *o;
        }
      }
      lua_pop(L, 2);
    }

    const char *msg = lua_pushfstring(L, "%s expected", type()->name());
    luaL_argerror(L, i, msg);
    abort(); // unreachable
  }
};

template<typename T>
struct LuaType<std::unique_ptr<T>> {
  using UT = std::unique_ptr<T>;

  static const LuaTypeInfo *type() {
    return &LuaTypeInfo::make<LuaType<UT>>();
  }

  static int gc(lua_State *L) {
    UT *o = (UT *) luaL_checkudata(L, 1, type()->name());
    o->~UT();
    return 0;
  }

  static void pushdata(lua_State *L, UT &o) {
    if (!o) {
      lua_pushnil(L);
      return;
    }

    void *u = lua_newuserdata(L, sizeof(UT));
    new(u) UT(std::move(o));
    luaL_getmetatable(L, type()->name());
    if (lua_isnil(L, -1)) {
      // If T is not registered,
      // registers a "__gc" to prevent memory leaks.
      lua_pop(L, 1);
      luaL_newmetatable(L, type()->name());
      lua_pushlightuserdata(L, (void *) type());
      lua_setfield(L, -2, "type");
      lua_pushcfunction(L, gc);
      lua_setfield(L, -2, "__gc");
    }
    lua_setmetatable(L, -2);
  }
};

// Generic Lua Object
template<>
struct LuaType<std::shared_ptr<LuaObj>> {
  static void pushdata(lua_State *L, std::shared_ptr<LuaObj> &o) {
    LuaObj::pushdata(L, o);
  }

  static std::shared_ptr<LuaObj> &todata(lua_State *L, int i, C_State *C) {
    return C->alloc<std::shared_ptr<LuaObj>>(LuaObj::todata(L, i));
  }
};

// C Function
template<>
struct LuaType<lua_CFunction> {
  static void pushdata(lua_State *L, lua_CFunction f) {
    lua_pushcfunction(L, f);
  }
};

// Bool
template<>
struct LuaType<bool> {
  static void pushdata(lua_State *L, bool o) {
    lua_pushboolean(L, o);
  }

  static bool todata(lua_State *L, int i, C_State * = NULL) {
    return lua_toboolean(L, i);
  }
};

// Integers
template<>
struct LuaType<int> {
  static void pushdata(lua_State *L, lua_Integer o) {
    lua_pushinteger(L, o);
  }

  static lua_Integer todata(lua_State *L, int i, C_State * = NULL) {
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

  static lua_Number todata(lua_State *L, int i, C_State * = NULL) {
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

  static std::string &todata(lua_State *L, int i, C_State *C) {
    return C->alloc<std::string>(luaL_checkstring(L, i));
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

  static std::vector<T> &todata(lua_State *L, int j, C_State *C) {
    auto &o = C->alloc<std::vector<T>>();
    int n = lua_rawlen(L, j);
    for (int i = 0; i < n; i++) {
      lua_rawgeti(L, j, i + 1);
      o.push_back(LuaType<T>::todata(L, -1, C));
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

  static std::set<T> &todata(lua_State *L, int j, C_State *C) {
    auto &o = C->alloc<std::set<T>>();
    o.clear();
    lua_pushnil(L);  /* first key */
    while (lua_next(L, j) != 0) {
      o.insert(LuaType<T>::todata(L, -2, C));
      lua_pop(L, 1);
    }
    return o;
  }
};

template<typename T>
struct LuaType<const std::vector<T>> : LuaType<std::vector<T>> {};

template<typename T>
struct LuaType<const std::vector<T> &> : LuaType<std::vector<T>> {};

template<>
struct LuaType<decltype(nullptr)> {
  static void pushdata(lua_State *L, decltype(nullptr) o) {
    lua_pushnil(L);
  }
};

// Helper function for pushing a series of data
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

template <typename O>
static LuaResult<O> todata_safe(lua_State *L, int i) {
  struct X {
    static int runner(lua_State *L) {
      O *po = (O *) lua_touserdata(L, 2);
      auto *C = (C_State *) lua_touserdata(L, 3);
      *po = LuaType<O>::todata(L, 1, C);
      return 0;
    }
  };

  O o;
  C_State C;
  lua_pushvalue(L, i);
  lua_pushcfunction(L, X::runner);
  lua_insert(L, -2);
  lua_pushlightuserdata(L, &o);
  lua_pushlightuserdata(L, &C);
  int status = lua_pcall(L, 3, 0, 0);

  if (status != LUA_OK) {
    std::string e = lua_tostring(L, -1);
    lua_pop(L, 1);
    return LuaResult<O>::Err({status, e});
  }
  return LuaResult<O>::Ok(o);
}

// --- Lua call/resume
template <typename ... I>
std::shared_ptr<LuaObj> Lua::newthread(I ... input) {
  pushdataX<I ...>(L_, input ...);
  return newthreadx(L_, sizeof...(input));
}

template <typename O>
LuaResult<O> Lua::resume(std::shared_ptr<LuaObj> f) {
  LuaObj::pushdata(L_, f);
  lua_State *C = lua_tothread(L_, -1);
  lua_pop(L_, 1);

  int status = xlua_resume(C, 0);
  if (status == LUA_YIELD) {
    auto r = todata_safe<O>(C, -1);
    lua_pop(C, 1);
    return r;
  } else {
    if (status != LUA_OK) {
      std::string e = lua_tostring(C, -1);
      lua_pop(C, 1);
      return LuaResult<O>::Err({status, e});
    }
    return LuaResult<O>::Err({status, ""});
  }
}

template <typename O, typename ... I>
LuaResult<O> Lua::call(I ... input) {
  pushdataX<I ...>(L_, input ...);

  int status = lua_pcall(L_, sizeof...(input) - 1, 1, 0);
  if (status != LUA_OK) {
    std::string e = lua_tostring(L_, -1);
    lua_pop(L_, 1);
    return LuaResult<O>::Err({status, e});
  }

  auto r = todata_safe<O>(L_, -1);
  lua_pop(L_, 1);
  return r;
}

template <typename ... I>
LuaResult<void> Lua::void_call(I ... input) {
  pushdataX<I ...>(L_, input ...);

  int status = lua_pcall(L_, sizeof...(input) - 1, 0, 0);
  if (status != LUA_OK) {
    std::string e = lua_tostring(L_, -1);
    lua_pop(L_, 1);
    return LuaResult<void>::Err({status, e});
  }

  return LuaResult<void>::Ok();
}

// --- LuaWrapper
// WRAP(f): wraps function f
// WRAPMEM(C::f): wraps member function C::f
// WRAPMEM_GET/SET(C::f): wraps member variable C::f

template<typename F, F f>
struct LUAWRAPPER_LOCAL LuaWrapper;

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
        static int wrap(lua_State *L, C_State *, Us... us) {
          R r = f(us...);
          LuaType<R>::pushdata(L, r);
          return 1;
        }
      };

      template<int n>
      struct ret<n, void> {
        static int wrap(lua_State *L, C_State *, Us... us) {
          f(us...);
          return 0;
        }
      };

      template<int n>
      static int wrap(lua_State *L, C_State *C, Us... us) {
        return ret<n, R>::wrap(L, C, us...);
      }
    };
  };

  // args = R :: A :: As
  template<typename R, typename A, typename... As>
  struct args<R, A, As...> {
    template<typename... Us>
    struct aux {
      template <int n>
      static int wrap(lua_State *L, C_State *C, Us... us) {
        A t = LuaType<A>::todata(L, n, C);
        return args<R, As...>::
          template aux<Us..., A &>::
          template wrap<n + 1>(L, C, us..., t);
      }
    };
  };

  static int wrap_helper(lua_State *L) {
    C_State *C = (C_State *) lua_touserdata(L, 1);
    return args<S, T...>::
      template aux<>::
      template wrap<2>(L, C);
  }

  static int wrap(lua_State *L) {
    return LuaImpl::wrap_common(L, wrap_helper);
  }
};

// MemberWrapper: R (C::*)(T..) -> R (C &, T...)
// MemberWrapperV(get variable): R (C::*) -> R (C &)
// MemberWrapperV(set variable): R (C::*) -> void (C &, R)
template<typename F, F f>
struct LUAWRAPPER_LOCAL MemberWrapper;

template<typename R, typename C, typename... T, R (C::*f)(T...)>
struct MemberWrapper<R (C::*)(T...), f> {
  template<typename D>
  static R wrapT(D &c, T... t) {
    return (c.*f)(t...);
  }

  static R wrap(C &c, T... t) {
    return wrapT<C>(c, t...);
  }
};

template<typename R, typename C, typename... T, R (C::*f)(T...) const>
struct MemberWrapper<R (C::*)(T...) const, f> {
  template<typename D>
  static R wrapT(const D &c, T... t) {
    return (c.*f)(t...);
  }

  static R wrap(const C &c, T... t) {
    return wrapT<C>(c, t...);
  }
};

#if __cplusplus >= 201703L || _MSVC_LANG >= 201703L
template<typename R, typename C, typename... T, R (C::*f)(T...) noexcept>
struct MemberWrapper<R (C::*)(T...) noexcept, f> {
  template<typename D>
  static R wrapT(D &c, T... t) {
    return (c.*f)(t...);
  }

  static R wrap(C &c, T... t) {
    return wrapT<C>(c, t...);
  }
};

template<typename R, typename C, typename... T, R (C::*f)(T...) const noexcept>
struct MemberWrapper<R (C::*)(T...) const noexcept, f> {
  template<typename D>
  static R wrapT(const D &c, T... t) {
    return (c.*f)(t...);
  }

  static R wrap(const C &c, T... t) {
    return wrapT<C>(c, t...);
  }
};
#endif

template<typename F, F f>
struct LUAWRAPPER_LOCAL MemberWrapperV;

template<typename R, typename C, R C::*f>
struct MemberWrapperV<R (C::*), f> {
  template<typename D>
  static R wrapT_get(const D &c) {
    return c.*f;
  }

  template<typename D>
  static void wrapT_set(D &c, R r) {
    c.*f = r;
  }

  static R wrap_get(const C &c) {
    return wrapT_get<C>(c);
  }

  static void wrap_set(C &c, R r) {
    return wrapT_set<C>(c, r);
  }
};

#define WRAP_PP_GET3(a1, a2, a3, ...) a3
#define WRAP_FIX_MSVC(...) __VA_ARGS__
#define WRAP_PP_CHOOSE(F, ...) WRAP_FIX_MSVC(WRAP_PP_GET3(__VA_ARGS__, F##_2, F##_1))

#define WRAP(f) (&(LuaWrapper<decltype(&f), &f>::wrap))
#define WRAPMEM_1(f) (&(LuaWrapper<decltype(&MemberWrapper<decltype(&f), &f>::wrap), \
                                            &MemberWrapper<decltype(&f), &f>::wrap>::wrap))
#define WRAPMEM_GET_1(f) (&(LuaWrapper<decltype(&MemberWrapperV<decltype(&f), &f>::wrap_get), \
                                                &MemberWrapperV<decltype(&f), &f>::wrap_get>::wrap))
#define WRAPMEM_SET_1(f) (&(LuaWrapper<decltype(&MemberWrapperV<decltype(&f), &f>::wrap_set), \
                                                &MemberWrapperV<decltype(&f), &f>::wrap_set>::wrap))
#define WRAPMEM_2(T, f) (&(LuaWrapper<decltype(&MemberWrapper<decltype(&T::f), &T::f>::wrapT<T>), \
                                               &MemberWrapper<decltype(&T::f), &T::f>::wrapT<T>>::wrap))
#define WRAPMEM_GET_2(T, f) (&(LuaWrapper<decltype(&MemberWrapperV<decltype(&T::f), &T::f>::wrapT_get<T>), \
                                                   &MemberWrapperV<decltype(&T::f), &T::f>::wrapT_get<T>>::wrap))
#define WRAPMEM_SET_2(T, f) (&(LuaWrapper<decltype(&MemberWrapperV<decltype(&T::f), &T::f>::wrapT_set<T>), \
                                                   &MemberWrapperV<decltype(&T::f), &T::f>::wrapT_set<T>>::wrap))
#define WRAPMEM(...) WRAP_FIX_MSVC(WRAP_PP_CHOOSE(WRAPMEM, __VA_ARGS__)(__VA_ARGS__))
#define WRAPMEM_GET(...) WRAP_FIX_MSVC(WRAP_PP_CHOOSE(WRAPMEM_GET, __VA_ARGS__)(__VA_ARGS__))
#define WRAPMEM_SET(...) WRAP_FIX_MSVC(WRAP_PP_CHOOSE(WRAPMEM_SET, __VA_ARGS__)(__VA_ARGS__))

#endif /* LIB_LUA_TEMPLATES_H_ */
