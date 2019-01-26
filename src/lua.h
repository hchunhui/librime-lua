#ifndef RIME_LUA_H_
#define RIME_LUA_H_

#include <rime/filter.h>
#include <rime/translator.h>
#include <rime/gear/filter_commons.h>

struct lua_State;

namespace rime {

class Lua {
  public:
  Lua();
  ~Lua();

  int newthread(lua_State *L, int nargs);

  template <typename ... I>
  int newthread(const string &f, I ... input);

  template <typename O>
  O resume(int id);

  static Lua *from_state(lua_State *L);
private:
  lua_State *L_;
};

}  // namespace rime

#endif  // RIME_LUA_H_
