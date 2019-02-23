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

  ~LuaTranslation() {
    lua_->unref(id_);
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

an<Translation> lua_translation_new(Lua *lua, int id) {
  return New<LuaTranslation>(lua, id);
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
