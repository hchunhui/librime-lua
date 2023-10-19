#ifndef LIB_LUA_H_
#define LIB_LUA_H_

#include <memory>
#include <functional>
#include <string>
#include "result.h"

struct lua_State;

class LuaObj {
public:
  ~LuaObj();

  static void pushdata(lua_State *L, std::shared_ptr<LuaObj> &o);
  static std::shared_ptr<LuaObj> todata(lua_State *L, int i);
  int type();

private:
  LuaObj(lua_State *L, int i);
  lua_State *L_;
  int id_;
};

struct LuaErr { int status; std::string e; };
template <typename T>
using LuaResult = Result<T, LuaErr>;

class Lua {
public:
  Lua();
  ~Lua();

  std::shared_ptr<LuaObj> getglobal(const std::string &f);

  std::shared_ptr<LuaObj> newthreadx(lua_State *L, int nargs);

  template <typename ... I>
  std::shared_ptr<LuaObj> newthread(I ... input);

  template <typename O>
  LuaResult<O> resume(std::shared_ptr<LuaObj> f);

  template <typename O, typename ... I>
  LuaResult<O> call(I ... input);

  template <typename ... I>
  LuaResult<void> void_call(I ... input);

  void to_state(std::function<void (lua_State *)> f);

  static Lua *from_state(lua_State *L);
private:
  lua_State *L_;
};

namespace LuaImpl {
  int wrap_common(lua_State *L, int (*cfunc)(lua_State *));
}

#endif  // LIB_LUA_H_
