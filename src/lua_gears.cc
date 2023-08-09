#include "lib/lua_templates.h"
#include "lua_gears.h"

namespace rime {

std::ostream &operator<<(std::ostream &os, const LuaErr &e) {
  return os << " error(" << e.status << "): " << e.e;
}
//--- LuaTranslation
bool LuaTranslation::Next() {
  if (exhausted()) {
    return false;
  }
  auto r = lua_->resume<an<Candidate>>(f_);
  if (!r.ok()) {
    LuaErr e = r.get_err();
    LOG_IF(ERROR, e.e != "" ) << typeid(*this).name() <<"::" << __FUNCTION__
                              << "[" << name_space_ << "]" << e;
    set_exhausted(true);
    return false;
  } else {
    c_ = r.get();
    return true;
  }
}

//---
static void raw_init(lua_State *L, const Ticket &t,
                     an<LuaObj> *env, an<LuaObj> *func, an<LuaObj> *fini, an<LuaObj> *tags_match= NULL) {
  lua_newtable(L);
  Engine *e = t.engine;
  LuaType<Engine *>::pushdata(L, e);
  lua_setfield(L, -2, "engine");
  LuaType<const string &>::pushdata(L, t.name_space);
  lua_setfield(L, -2, "name_space");
  *env = LuaObj::todata(L, -1);
  lua_pop(L, 1);

  if (t.klass.size() > 0 && t.klass[0] == '*') {
    lua_getglobal(L, "require");
    lua_pushstring(L, t.klass.c_str() + 1);
    int status = lua_pcall(L, 1, 1, 0);
    if (status != LUA_OK) {
      const char *e = lua_tostring(L, -1);
      LOG(ERROR) << "Lua Compoment of autoload error:("
                 << " module: "<< t.klass
                 << " name_space: " << t.name_space
                 << " status: " << status
                 << " ): " << e;
    }
  } else {
    lua_getglobal(L, t.klass.c_str());
  }

  if (lua_type(L, -1) == LUA_TTABLE) {
    lua_getfield(L, -1, "init");
    if (lua_type(L, -1) == LUA_TFUNCTION) {
      LuaObj::pushdata(L, *env);
      int status = lua_pcall(L, 1, 1, 0);
      if (status != LUA_OK) {
        const char *e = lua_tostring(L, -1);
        LOG(ERROR) << "Lua Compoment of initialize  error:("
          << " module: "<< t.klass
          << " name_space: " << t.name_space
          << " status: " << status
          << " ): " << e;
      }
    }
    lua_pop(L, 1);

    lua_getfield(L, -1, "fini");
    if (lua_type(L, -1) == LUA_TFUNCTION) {
      *fini = LuaObj::todata(L, -1);
    }
    lua_pop(L, 1);

    if (tags_match) {
      lua_getfield(L, -1, "tags_match");
      if (lua_type(L, -1) == LUA_TFUNCTION) {
        *tags_match = LuaObj::todata(L, -1);
      }
      lua_pop(L, 1);
    }

    lua_getfield(L, -1, "func");
  }

  if (lua_type(L, -1) != LUA_TFUNCTION) {
    LOG(ERROR) << "Lua Compoment of initialize  error:("
      << " module: "<< t.klass
      << " name_space: " << t.name_space
      << " func type: " << luaL_typename(L, -1)
      << " ): " << "func type error expect function ";
  }
  *func = LuaObj::todata(L, -1);
  lua_pop(L, 1);
}
#define VOID_CALL(func, fname)                                                      \
do {                                                                                \
  if (func) {                                                                       \
    auto r = lua_->void_call<an<LuaObj>, an<LuaObj>>(fini_, env_);                  \
    LOG_IF(ERROR, !r.ok()) << typeid(*this).name() <<"::" << __FUNCTION__           \
                           << "[" << name_space_ << ": " #fname "]" << r.get_err(); \
  }                                                                                 \
} while(0)

//--- LuaFilter
LuaFilter::LuaFilter(const Ticket& ticket, Lua* lua)
  : Filter(ticket), TagMatching(ticket), lua_(lua) {
  lua->to_state([&](lua_State *L) {raw_init(L, ticket, &env_, &func_, &fini_, &tags_match_);});
}

bool LuaFilter::AppliesToSegment(Segment* segment) {
  if ( ! tags_match_ )
    return TagsMatch(segment);

  auto r = lua_->call<bool, an<LuaObj>, Segment *, an<LuaObj>>(tags_match_, segment,  env_);
  LOG_IF(ERROR, !r.ok()) << typeid(*this).name() << "::" << __FUNCTION__
                         << "[" << name_space_ << "]" << r.get_err();
  return (r.ok()) ? r.get() : false;
}

an<Translation> LuaFilter::Apply(
  an<Translation> translation, CandidateList* candidates) {
  auto f = lua_->newthread<an<LuaObj>, an<Translation>,
                           an<LuaObj>, CandidateList *>(func_, translation, env_, candidates);
  return New<LuaTranslation>(lua_, f, name_space_ + ":\"func\"");
}

LuaFilter::~LuaFilter() { VOID_CALL(fini_, "fini"); }

//--- LuaTranslator
LuaTranslator::LuaTranslator(const Ticket& ticket, Lua* lua)
  : Translator(ticket), lua_(lua) {
  lua->to_state([&](lua_State *L) {raw_init(L, ticket, &env_, &func_, &fini_);});
}

an<Translation> LuaTranslator::Query(const string& input,
                                     const Segment& segment) {
  auto f = lua_->newthread<an<LuaObj>, const string &, const Segment &,
                           an<LuaObj>>(func_, input, segment, env_);
  an<Translation> t = New<LuaTranslation>(lua_, f, name_space_ + ": \"func\"");
  if (t->exhausted())
    return an<Translation>();
  else
    return t;
}

LuaTranslator::~LuaTranslator() { VOID_CALL(fini_, "fini"); }

//--- LuaSegmentor
LuaSegmentor::LuaSegmentor(const Ticket& ticket, Lua *lua)
  : Segmentor(ticket), lua_(lua) {
  lua->to_state([&](lua_State *L) {raw_init(L, ticket, &env_, &func_, &fini_);});
}

bool LuaSegmentor::Proceed(Segmentation* segmentation) {
  auto r = lua_->call<bool, an<LuaObj>, Segmentation &,
                      an<LuaObj>>(func_, *segmentation, env_);
  LOG_IF(ERROR, !r.ok()) << typeid(*this).name() << "::" << __FUNCTION__
                         << "[" << name_space_ << "]" << r.get_err();
  return (r.ok()) ? r.get() : true ;
}

LuaSegmentor::~LuaSegmentor() { VOID_CALL(fini_, "fini"); }

//--- LuaProcessor
LuaProcessor::LuaProcessor(const Ticket& ticket, Lua* lua)
  : Processor(ticket), lua_(lua) {
  lua->to_state([&](lua_State *L) {raw_init(L, ticket, &env_, &func_, &fini_);});
}

ProcessResult LuaProcessor::ProcessKeyEvent(const KeyEvent& key_event) {
  auto r = lua_->call<int, an<LuaObj>, const KeyEvent&,
                      an<LuaObj>>(func_, key_event, env_);
  if (!r.ok()) {
    LOG_IF(ERROR, !r.ok()) << typeid(*this).name() << "::" << __FUNCTION__
                           << "[" << name_space_ << "]" << r.get_err();
    return kNoop;
  } else
    switch (r.get()) {
    case 0: return kRejected;
    case 1: return kAccepted;
    default: return kNoop;
    }
}

LuaProcessor::~LuaProcessor() { VOID_CALL(fini_, "fini"); }

}  // namespace rime
