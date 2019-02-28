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
  an<LuaObj> f_;
public:
  LuaTranslation(Lua *lua, an<LuaObj> f)
    : lua_(lua), f_(f) {
    Next();
  }

  bool Next() {
    if (exhausted()) {
      return false;
    }
    auto r = lua_->resume<an<Candidate>>(f_);
    if (!r) {
      set_exhausted(true);
      return false;
    } else {
      c_ = *r;
      return true;
    }
  }

  an<Candidate> Peek() {
    return c_;
  }
};

an<Translation> lua_translation_new(Lua *lua, an<LuaObj> o) {
  return New<LuaTranslation>(lua, o);
}

//--- LuaFilter
LuaFilter::LuaFilter(const Ticket& ticket, Lua* lua)
  : Filter(ticket), TagMatching(ticket), lua_(lua) {
  f_ = lua->getglobal<an<LuaObj>>(ticket.name_space);
}

an<Translation> LuaFilter::Apply(
  an<Translation> translation, CandidateList* candidates) {
  auto f = lua_->newthread<an<LuaObj>, an<Translation>>(f_, translation);
  return New<LuaTranslation>(lua_, f);
}

//--- LuaTranslator
LuaTranslator::LuaTranslator(const Ticket& ticket, Lua* lua)
  : Translator(ticket), lua_(lua) {
  f_ = lua->getglobal<an<LuaObj>>(ticket.name_space);
}

an<Translation> LuaTranslator::Query(const string& input,
                                     const Segment& segment) {
  auto f = lua_->newthread<an<LuaObj>, const string &, const Segment &>(f_, input, segment);
  an<Translation> t = New<LuaTranslation>(lua_, f);
  if (t->exhausted())
    return an<Translation>();
  else
    return t;
}

}  // namespace rime
