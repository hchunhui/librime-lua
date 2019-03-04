#ifndef RIME_LUA_H_
#define RIME_LUA_H_

#include <functional>
#include <rime/filter.h>
#include <rime/translator.h>
#include <rime/gear/filter_commons.h>

struct lua_State;

namespace rime {

class LuaObj {
public:
  ~LuaObj();

  static void pushdata(lua_State *L, an<LuaObj> &o);
  static an<LuaObj> todata(lua_State *L, int i);

private:
  LuaObj(lua_State *L, int i);
  lua_State *L_;
  int id_;
};

class Lua {
public:
  Lua();
  ~Lua();

  template <typename O>
  O getglobal(const string &f);

  an<LuaObj> newthreadx(lua_State *L, int nargs);

  template <typename ... I>
  an<LuaObj> newthread(I ... input);

  template <typename O>
  optional<O> resume(an<LuaObj> f);

  template <typename O, typename ... I>
  O call(I ... input);

  void to_state(std::function<void (lua_State *)> f);

  static Lua *from_state(lua_State *L);
private:
  lua_State *L_;
};

}  // namespace rime

#endif  // RIME_LUA_H_
