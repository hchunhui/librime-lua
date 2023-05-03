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

  // try to call init(env) function
  if (lua_type(L, -1) == LUA_TTABLE) {
    // call init(env) if exist  init function
    lua_getfield(L, -1, "init");
    if (lua_type(L, -1) == LUA_TFUNCTION) {
      LuaObj::pushdata(L, env);
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
    if ( lua_getfield(L, -1, it.first.c_str()) == LUA_TFUNCTION ) {
      *(it.second) = LuaObj::todata(L, -1);
    }
    else {
      LOG(INFO) << "Lua Compoment of initialize warning :"
        << " module: "<< t.klass
        << ", name_space: " << t.name_space
        << ", function: " << it.first << " is " <<  lua_typename(L, -1) << " (function expected)." ;
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
  if (! func_ && apply_ ){
    LOG(ERROR) <<  "LuaFilterr initialize failed of " << ticket.klass + "@" + ticket.name_space
      << " : function not found (func or query).";
  }

}

an<Translation> LuaFilter::LuaApply(
    an<Translation> translation, CandidateList* candidates) {
  //  apply(inp, cands, env) return translation
  auto r= lua_->call<an<Translation>, an<LuaObj>, an<Translation>, an<LuaObj>, CandidateList*>
    (apply_, translation, env_, candidates);
  if (!r.ok()) {
    auto e = r.get_err();
    LOG(ERROR) << "LuaFilter::LuaApply(apply) of "<< name_space_ << " error(" << e.status << "): " << e.e;
    return New<CacheTranslation>(nullptr);
  }
  else
    return r.get();
}

an<Translation> LuaFilter::Apply(
  an<Translation> translation, CandidateList* candidates) {
  if (apply_){
    return LuaApply(translation, candidates);
  }
  else if (func_){
    auto f = lua_->newthread<an<LuaObj>, an<Translation>,
         an<LuaObj>, CandidateList *>(func_, translation, env_, candidates);
    return New<LuaTranslation>(lua_, f);
  }
  else {
    LOG(ERROR) << "LuaFilter of " << name_space_ << " function not found ( apply or func ).";
    return New<CacheTranslation>(nullptr);
  }
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
    {"query", &query_},
  };
  lua->to_state( [&](lua_State *L) { raw_init(L, ticket, env_, table); } );
  if (! func_ && !query_ ){
    LOG(ERROR) <<  "LuaTranslator initialize failed of " << ticket.klass + "@" + ticket.name_space
      << ": function not found (func or apply).";
  }
}

an<Translation> LuaTranslator::LuaQuery(const string& input,
    const Segment& segment) {
  //  query(inp, seg, env) return translation
  auto r= lua_->call<an<Translation>, an<LuaObj>, const string &, const Segment &, an<LuaObj> >
    (query_, input, segment, env_);
  if (!r.ok()) {
    auto e = r.get_err();
    LOG(ERROR) << "LuaTranslator::LuaQuery(query) of "<< name_space_ << " error(" << e.status << "): " << e.e;
    return an<Translation>();
  }
  else
    return r.get();
}

an<Translation> LuaTranslator::Query(const string& input,
                                     const Segment& segment) {
  an<Translation> t;
  if (query_){
    t = LuaQuery(input, segment);
  }
  else if (func_) {
    auto f = lua_->newthread<an<LuaObj>, const string &, const Segment &,
      an<LuaObj>>(func_, input, segment, env_);
    t = New<LuaTranslation>(lua_, f);
  }
  else {
    LOG(ERROR) <<  "LuaTranslator namespace of " << name_space_
      << " : function not found (func or apply).";
  }
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
  if (! func_ ){
    LOG(ERROR) <<  "LuaSegmentor initialize failed of " << ticket.klass + "@" + ticket.name_space
      << ": function not found ( func ).";
  }
}

bool LuaSegmentor::Proceed(Segmentation* segmentation) {
  if (! func_ ){
    LOG(ERROR) <<  "LuaSegmentor::Processed of " << name_space_
      << ": function not found ( func ).";
    return true;
  }
  auto r = lua_->call<bool, an<LuaObj>, Segmentation &,
                      an<LuaObj>>(func_, *segmentation, env_);
  if (!r.ok()) {
    auto e = r.get_err();
    LOG(ERROR) << "LuaSegmentor::Proceed(func) of "<< name_space_ << " error(" << e.status << "): " << e.e;
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
  if (! func_ ){
    LOG(ERROR) <<  "LuaProcessor initialize failed of " << ticket.klass + "@" + ticket.name_space
      << ": function not found ( func ).";
  }
}

ProcessResult LuaProcessor::ProcessKeyEvent(const KeyEvent& key_event) {
  if (! func_ ){
    LOG(ERROR) <<  "LuaProcessorr::ProcessKeyEvent of " << name_space_
      << ": function not found ( func ).";
    return kNoop;
  }
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
