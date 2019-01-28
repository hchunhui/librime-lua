#include <rime/engine.h>
#include <rime/segmentation.h>
#include <rime/translation.h>
#include "lua.h"
#include "lua_templates.h"
#include "lua_gears.h"

namespace rime {

//--- LuaTranslation
class LuaTranslation : public Translation {
public:
  Lua *lua_;
private:
  an<Candidate> c_;
  int id_;
public:
  LuaTranslation(Lua *lua, int id)
    : lua_(lua), id_(id) {
    Next();
  }

  bool Next() {
    if (exhausted()) {
      return false;
    }
    c_ = lua_->resume<an<Candidate>>(id_);
    if (!c_) {
      set_exhausted(true);
      return false;
    } else {
      return true;
    }
  }

  an<Candidate> Peek() {
    return c_;
  }
};

int raw_lua_translation(lua_State *L) {
  Lua *lua = Lua::from_state(L);
  int n = lua_gettop(L);

  if (n < 1)
    return 0;

  int id = lua->newthread(L, n - 1);
  auto r = New<LuaTranslation>(lua, id);
  LuaType<an<Translation>>::pushdata(L, r);
  return 1;
}

//--- LuaFilter
LuaFilter::LuaFilter(const Ticket& ticket, Lua* lua)
  : Filter(ticket), TagMatching(ticket), fname_(ticket.name_space), lua_(lua) {
}

an<Translation> LuaFilter::Apply(
  an<Translation> translation, CandidateList* candidates) {
  int id = lua_->newthread<an<Translation>>(fname_.c_str(), translation);
  return New<LuaTranslation>(lua_, id);
}

//--- LuaTranslator
LuaTranslator::LuaTranslator(const Ticket& ticket, Lua* lua)
  : Translator(ticket), fname_(ticket.name_space), lua_(lua) {
}

an<Translation> LuaTranslator::Query(const string& input,
                                     const Segment& segment) {
  int id = lua_->newthread<const string &, const Segment &>(fname_.c_str(), input, segment);
  an<Translation> t = New<LuaTranslation>(lua_, id);
  if (t->exhausted())
    return an<Translation>();
  else
    return t;
}

}  // namespace rime
