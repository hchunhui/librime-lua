#include "lib/lua_templates.h"
#include "lua_gears.h"

namespace rime {

//--- LuaTranslation
bool LuaTranslation::Next() {
  if (exhausted()) {
    return false;
  }
  auto r = lua_->resume<an<Candidate>>(f_);
  if (!r.ok()) {
    LuaErr e = r.get_err();
    if (e.e != "")
      LOG(ERROR) << "LuaTranslation::Next error(" << e.status << "): " << e.e;
    set_exhausted(true);
    return false;
  } else {
    c_ = r.get();
    return true;
  }
}

//---
static void raw_init(lua_State *L, const Ticket &t,
                     an<LuaObj> *env, an<LuaObj> *func, an<LuaObj> *fini) {
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
        LOG(ERROR) << "Lua error:(" << status << "): " << e;
      }
    }
    lua_pop(L, 1);

    lua_getfield(L, -1, "fini");
    if (lua_type(L, -1) == LUA_TFUNCTION) {
      *fini = LuaObj::todata(L, -1);
    }
    lua_pop(L, 1);

    lua_getfield(L, -1, "func");
  }

  *func = LuaObj::todata(L, -1);
  lua_pop(L, 1);
}

static void raw_tags_match(lua_State *L, const Ticket &t, an<LuaObj> *tags_match) {
  lua_getglobal(L, t.klass.c_str());
  if (lua_type(L, -1) == LUA_TTABLE) {
    lua_getfield(L, -1, "tags_match");
    if (lua_type(L, -1) == LUA_TFUNCTION) {
      *tags_match = LuaObj::todata(L, -1);
    }
    else {
      *tags_match = nullptr;
    }
    lua_pop(L, 1);
  }
  lua_pop(L, 1);
}

//--- LuaFilter
LuaFilter::LuaFilter(const Ticket& ticket, Lua* lua)
  : Filter(ticket), TagMatching(ticket), lua_(lua) {
  lua->to_state([&](lua_State *L) {raw_init(L, ticket, &env_, &func_, &fini_);});
  // initilize lua function  : tags_match( segment, env)
  lua->to_state([&](lua_State *L) {raw_tags_match(L, ticket, &tags_match_);});
}

an<Translation> LuaFilter::Apply(
  an<Translation> translation, CandidateList* candidates) {
  auto f = lua_->newthread<an<LuaObj>, an<Translation>,
                           an<LuaObj>>(func_, translation, env_);
  return New<LuaTranslation>(lua_, f);
}

LuaFilter::~LuaFilter() {
  if (fini_) {
    auto r = lua_->void_call<an<LuaObj>, an<LuaObj>>(fini_, env_);
    if (!r.ok()) {
      auto e = r.get_err();
      LOG(ERROR) << "LuaFilter::~LuaFilter error(" << e.status << "): " << e.e;
    }
  }
}

//--- LuaTranslator
LuaTranslator::LuaTranslator(const Ticket& ticket, Lua* lua)
  : Translator(ticket), lua_(lua) {
  lua->to_state([&](lua_State *L) {raw_init(L, ticket, &env_, &func_, &fini_);});
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

LuaTranslator::~LuaTranslator() {
  if (fini_) {
    auto r = lua_->void_call<an<LuaObj>, an<LuaObj>>(fini_, env_);
    if (!r.ok()) {
      auto e = r.get_err();
      LOG(ERROR) << "LuaTranslator::~LuaTranslator error(" << e.status << "): " << e.e;
    }
  }
}

//--- LuaSegmentor
LuaSegmentor::LuaSegmentor(const Ticket& ticket, Lua *lua)
  : Segmentor(ticket), lua_(lua) {
  lua->to_state([&](lua_State *L) {raw_init(L, ticket, &env_, &func_, &fini_);});
}

bool LuaSegmentor::Proceed(Segmentation* segmentation) {
  auto r = lua_->call<bool, an<LuaObj>, Segmentation &,
                      an<LuaObj>>(func_, *segmentation, env_);
  if (!r.ok()) {
    auto e = r.get_err();
    LOG(ERROR) << "LuaSegmentor::Proceed error(" << e.status << "): " << e.e;
    return true;
  } else
    return r.get();
}

LuaSegmentor::~LuaSegmentor() {
  if (fini_) {
    auto r = lua_->void_call<an<LuaObj>, an<LuaObj>>(fini_, env_);
    if (!r.ok()) {
      auto e = r.get_err();
      LOG(ERROR) << "LuaSegmentor::~LuaSegmentor error(" << e.status << "): " << e.e;
    }
  }
}

//--- LuaProcessor
LuaProcessor::LuaProcessor(const Ticket& ticket, Lua* lua)
  : Processor(ticket), lua_(lua) {
  lua->to_state([&](lua_State *L) {raw_init(L, ticket, &env_, &func_, &fini_);});
}

ProcessResult LuaProcessor::ProcessKeyEvent(const KeyEvent& key_event) {
  auto r = lua_->call<int, an<LuaObj>, const KeyEvent&,
                      an<LuaObj>>(func_, key_event, env_);
  if (!r.ok()) {
    auto e = r.get_err();
    LOG(ERROR) << "LuaProcessor::ProcessKeyEvent error(" << e.status << "): " << e.e;
    return kNoop;
  } else
    switch (r.get()) {
    case 0: return kRejected;
    case 1: return kAccepted;
    default: return kNoop;
    }
}

LuaProcessor::~LuaProcessor() {
  if (fini_) {
    auto r = lua_->void_call<an<LuaObj>, an<LuaObj>>(fini_, env_);
    if (!r.ok()) {
      auto e = r.get_err();
      LOG(ERROR) << "LuaProcessor::~LuaProcessor error(" << e.status << "): " << e.e;
    }
  }
}

}  // namespace rime
