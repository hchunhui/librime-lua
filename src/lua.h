#ifndef RIME_LUA_H_
#define RIME_LUA_H_

#include <memory>
#include <functional>
#include <string>
#include <boost/optional.hpp>

struct lua_State;

class LuaObj {
public:
  ~LuaObj();

  static void pushdata(lua_State *L, std::shared_ptr<LuaObj> &o);
  static std::shared_ptr<LuaObj> todata(lua_State *L, int i);

private:
  LuaObj(lua_State *L, int i);
  lua_State *L_;
  int id_;
};

class Lua {
public:
  Lua(const std::string &init_file);
  ~Lua();

  template <typename O>
  O getglobal(const std::string &f);

  std::shared_ptr<LuaObj> newthreadx(lua_State *L, int nargs);

  template <typename ... I>
  std::shared_ptr<LuaObj> newthread(I ... input);

  template <typename O>
  boost::optional<O> resume(std::shared_ptr<LuaObj> f);

  template <typename O, typename ... I>
  O call(I ... input);

  void to_state(std::function<void (lua_State *)> f);

  static Lua *from_state(lua_State *L);
private:
  lua_State *L_;
};

#endif  // RIME_LUA_H_
