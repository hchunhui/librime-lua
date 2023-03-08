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
typedef std::map<string,an<LuaObj>*> InitMap;

static void raw_init(lua_State *L, const Ticket &t, an<LuaObj> &env, InitMap &table ) {
  // init env table
  Engine *e = t.engine;
  lua_newtable(L);
  LuaType<Engine *>::pushdata(L, e);
  lua_setfield(L, -2, "engine");
  LuaType<const string &>::pushdata(L, t.name_space);
  lua_setfield(L, -2, "name_space");
  env = LuaObj::todata(L, -1);
  lua_pop(L, 1);

  // load lua module
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

  // try to init(env) function
  if (lua_type(L, -1) == LUA_TTABLE) {
    // call init(env) if exist  init function
    lua_getfield(L, -1, "init");
    if (lua_type(L, -1) == LUA_TFUNCTION) {
      LuaObj::pushdata(L, env);
      int status = lua_pcall(L, 1, 0, 0);
      if (status != LUA_OK) {
        const char *e = lua_tostring(L, -1);
        LOG(ERROR) << "Lua Compoment of initialize  error:("
          << " module: "<< t.klass
          << " name_space: " << t.name_space
          << " status: " << status
          << " ): " << e;
      }
    }
  } else if (lua_type(L, -1) == LUA_TFUNCTION) {
    // create table { func = func}
    lua_newtable(L);
    lua_insert(L, -2);
    lua_setfield(L, -2, "func");
  } else {
    lua_pop(L, lua_gettop(L));
    return ;
  }

  // init  nadel of func
  for (auto it: table) {
    if ( lua_getfield(L, -1, it.first.c_str()) == LUA_TFUNCTION){
      *it.second = LuaObj::todata(L, -1);
    }
    lua_pop(L,1);
  }
  lua_pop(L, lua_gettop(L));
}

//--- LuaFilter
LuaFilter::LuaFilter(const Ticket& ticket, Lua* lua)
  : Filter(ticket), TagMatching(ticket), lua_(lua) {
  InitMap table= {
    {"func", &func_},
    {"fini", &fini_},
    {"tags_match", &tags_match_ },
    {"apply", &apply_ },
  };
  lua->to_state( [&](lua_State *L) { raw_init(L, ticket, env_, table); } );
}

an<Translation> LuaFilter::Apply(
  an<Translation> translation, CandidateList* candidates) {

  auto f = lua_->newthread<an<LuaObj>, an<Translation>,
                           an<LuaObj>, CandidateList *>(func_, translation, env_, candidates);
  return New<LuaTranslation>(lua_, f);
}

LuaFilter::~LuaFilter() {
  if (fini_) {
    auto r = lua_->void_call<an<LuaObj>, an<LuaObj>>(fini_, env_);
    if (!r.ok()) {
      auto e = r.get_err();
      LOG(ERROR) << "LuaFilter::~LuaFilter of "<< name_space_ << " error(" << e.status << "): " << e.e;
    }
  }
}

//--- LuaTranslator
LuaTranslator::LuaTranslator(const Ticket& ticket, Lua* lua)
  : Translator(ticket), lua_(lua) {
  InitMap table= {
    {"func", &func_},
    {"fini", &fini_},
  };
  lua->to_state( [&](lua_State *L) { raw_init(L, ticket, env_, table); } );
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
      LOG(ERROR) << "LuaTranslator::~LuaTranslator of "<< name_space_ << " error(" << e.status << "): " << e.e;
    }
  }
}

//--- LuaSegmentor
LuaSegmentor::LuaSegmentor(const Ticket& ticket, Lua *lua)
  : Segmentor(ticket), lua_(lua) {
  InitMap table= {
    {"func", &func_},
    {"fini", &fini_},
  };
  lua->to_state( [&](lua_State *L) { raw_init(L, ticket, env_, table); } );
}

bool LuaSegmentor::Proceed(Segmentation* segmentation) {
  auto r = lua_->call<bool, an<LuaObj>, Segmentation &,
                      an<LuaObj>>(func_, *segmentation, env_);
  if (!r.ok()) {
    auto e = r.get_err();
    LOG(ERROR) << "LuaSegmentor::Proceed of "<< name_space_ << " error(" << e.status << "): " << e.e;
    return true;
  } else
    return r.get();
}

LuaSegmentor::~LuaSegmentor() {
  if (fini_) {
    auto r = lua_->void_call<an<LuaObj>, an<LuaObj>>(fini_, env_);
    if (!r.ok()) {
      auto e = r.get_err();
      LOG(ERROR) << "LuaSegmentor::~LuaSegmentor of "<< name_space_ << " error(" << e.status << "): " << e.e;
    }
  }
}

//--- LuaProcessor
LuaProcessor::LuaProcessor(const Ticket& ticket, Lua* lua)
  : Processor(ticket), lua_(lua) {
  InitMap table= {
    {"func", &func_},
    {"fini", &fini_},
  };
  lua->to_state( [&](lua_State *L) { raw_init(L, ticket, env_, table); } );
}

ProcessResult LuaProcessor::ProcessKeyEvent(const KeyEvent& key_event) {
  auto r = lua_->call<int, an<LuaObj>, const KeyEvent&,
                      an<LuaObj>>(func_, key_event, env_);
  if (!r.ok()) {
    auto e = r.get_err();
    LOG(ERROR) << "LuaProcessor::ProcessKeyEvent of "<< name_space_ << " error(" << e.status << "): " << e.e;
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
      LOG(ERROR) << "LuaProcessor::~LuaProcessor of "<< name_space_ << " error(" << e.status << "): " << e.e;
    }
  }
}

}  // namespace rime
