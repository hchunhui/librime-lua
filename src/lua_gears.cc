#include <rime/engine.h>
#include <rime/segmentation.h>
#include <rime/translation.h>
#include <rime/key_event.h>
#include "lib/lua_templates.h"
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

static void raw_init(lua_State *L, const Ticket &t,
                     an<LuaObj> *env, an<LuaObj> *func) {
  lua_newtable(L);
  Engine *e = t.engine;
  LuaType<Engine *>::pushdata(L, e);
  lua_setfield(L, -2, "engine");
  LuaType<const string &>::pushdata(L, t.name_space);
  lua_setfield(L, -2, "name_space");
  *env = LuaObj::todata(L, -1);
  lua_pop(L, 1);

  lua_getglobal(L, t.klass.c_str());
  if (lua_type(L, -1) == LUA_TTABLE) {
    lua_getfield(L, -1, "init");
    if (lua_type(L, -1) == LUA_TFUNCTION) {
      LuaObj::pushdata(L, *env);
      int status = lua_pcall(L, 1, 1, 0);
      if (status != LUA_OK) {
        const char *e = lua_tostring(L, -1);
        printf("call(err=%d): %s\n", status, e);
      }
    }
    lua_pop(L, 1);
    lua_getfield(L, -1, "func");
  }

  *func = LuaObj::todata(L, -1);
  lua_pop(L, 1);
}

//--- LuaFilter
LuaFilter::LuaFilter(const Ticket& ticket, Lua* lua)
  : Filter(ticket), TagMatching(ticket), lua_(lua) {
  lua->to_state([&](lua_State *L) {raw_init(L, ticket, &env_, &func_);});
}

an<Translation> LuaFilter::Apply(
  an<Translation> translation, CandidateList* candidates) {
  auto f = lua_->newthread<an<LuaObj>, an<Translation>,
                           an<LuaObj>>(func_, translation, env_);
  return New<LuaTranslation>(lua_, f);
}

//--- LuaTranslator
LuaTranslator::LuaTranslator(const Ticket& ticket, Lua* lua)
  : Translator(ticket), lua_(lua) {
  lua->to_state([&](lua_State *L) {raw_init(L, ticket, &env_, &func_);});
}

an<Translation> LuaTranslator::Query(const string& input,
                                     const Segment& segment) {
  auto f = lua_->newthread<an<LuaObj>, const string &, const Segment &,
                           an<LuaObj>>(func_, input, segment, env_);
  an<Translation> t = New<LuaTranslation>(lua_, f);
  if (t->exhausted())
    return an<Translation>();
  else
    return t;
}

//--- LuaSegmentor
LuaSegmentor::LuaSegmentor(const Ticket& ticket, Lua *lua)
  : Segmentor(ticket), lua_(lua) {
  lua->to_state([&](lua_State *L) {raw_init(L, ticket, &env_, &func_);});
}

bool LuaSegmentor::Proceed(Segmentation* segmentation) {
  return lua_->call<bool, an<LuaObj>, Segmentation &,
                    an<LuaObj>>(func_, *segmentation, env_);
}

//--- LuaProcessor
LuaProcessor::LuaProcessor(const Ticket& ticket, Lua* lua)
  : Processor(ticket), lua_(lua) {
  lua->to_state([&](lua_State *L) {raw_init(L, ticket, &env_, &func_);});
}

ProcessResult LuaProcessor::ProcessKeyEvent(const KeyEvent& key_event) {
  int r = lua_->call<int, an<LuaObj>, const KeyEvent&,
                     an<LuaObj>>(func_, key_event, env_);
  switch (r) {
  case 0: return kRejected;
  case 1: return kAccepted;
  default: return kNoop;
  }
}

}  // namespace rime
