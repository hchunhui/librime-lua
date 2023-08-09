#include "lib/lua_templates.h"
#include "lua_gears.h"

namespace rime {
#define LUA_TANY -2

struct InitTab {
  string name;
  an<LuaObj>* luaobj;
  int type;//lua type : LUA_TFUNCTION ...
};

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
//(-0,+1,+m)
static void raw_init_env(lua_State *L, const Ticket &t, an<LuaObj> *env) {
  lua_newtable(L);
  Engine *e = t.engine;
  LuaType<Engine *>::pushdata(L, e);
  lua_setfield(L, -2, "engine");
  LuaType<const string &>::pushdata(L, t.name_space);
  lua_setfield(L, -2, "name_space");
  *env = LuaObj::todata(L, -1);
  lua_pop(L, 1);
}

//(-0, +1) table
static bool raw_load_module(lua_State *L, const Ticket &t) {
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

  int ltype = lua_type(L, -1);
  if ( ltype == LUA_TTABLE) {
    return true;
  } else if ( ltype == LUA_TFUNCTION) {
    lua_newtable(L);
    lua_rotate(L, -2, 1);
    lua_setfield(L, -2, "func");
    return true;
  } else {
    LOG(ERROR) << "loading module [" << t.klass << "@" << t.name_space << "]"
	       << luaL_typename(L, -1)
	       << " error:() type error function or table expected ";
    lua_newtable(L);
    lua_rotate(L, -2, 1);
    lua_setfield(L, -2, "func"); // LUA_TANY
    return false;
  }
}
// (-0, +0, +m) regist InitTab.luaobj
static void reg_funcs(lua_State *L, int index, InitTab *luaobjs, size_t size) {
  for (int i=0; i < size; i++) {
    auto t = luaobjs[i];
    int ltype = lua_getfield(L, index, t.name.c_str());
    if ( t.type == LUA_TANY || t.type == ltype) {
      *(t.luaobj) = LuaObj::todata(L, -1);
    }
    lua_pop(L, 1);
  }
}
//---
// (-0, +0, +m) update luaobjs
static void raw_init(lua_State *L, const Ticket &t,
                     an<LuaObj> *env, InitTab *luaobjs, size_t size) {
  raw_init_env(L, t, env);//(-0, +0, +m) update env
  raw_load_module(L, t);//(-0, +1)  table
  reg_funcs(L, -1, luaobjs, size);//(-0,+0,+m)  luaobjs
  lua_pop(L, 1); // remove module table
}
  // ex: VOID_CALL(init_, "init")
#define VOID_CALL(func, str_fname)					\
  do {									\
    if (func) {								\
      auto r = lua_->void_call<an<LuaObj>, an<LuaObj>>(func, env_);	\
      if (!r.ok()) {							\
	auto e = r.get_err();						\
	LOG(ERROR) << typeid(*this).name() << "::" << __FUNCTION__	\
		   << " [" << name_space_ << ": " #str_fname " ] "	\
		   << " error(" << e.status << "): " << e.e;		\
      }									\
    }									\
  } while(0)

#define F_FIELD(name) { #name, &name##_, LUA_TFUNCTION }
#define A_FIELD(name) { #name, &name##_, LUA_TANY }

#define DEF_FIELD()				\
  {"init", &init, LUA_TFUNCTION },		\
    F_FIELD(fini),				\
    A_FIELD(func)



//--- LuaFilter
LuaFilter::LuaFilter(const Ticket& ticket, Lua* lua)
  : Filter(ticket), TagMatching(ticket), lua_(lua) {
  an<LuaObj> init;
  InitTab luaobjs[]= { DEF_FIELD(), F_FIELD(tags_match), };
  size_t size = sizeof(luaobjs) / sizeof(InitTab);

  lua->to_state([&](lua_State *L) {raw_init(L, ticket, &env_, luaobjs, size );});
  VOID_CALL(init, "init");
}


an<Translation> LuaFilter::Apply(
  an<Translation> translation, CandidateList* candidates) {
  auto f = lua_->newthread<an<LuaObj>, an<Translation>,
                           an<LuaObj>, CandidateList *>(func_, translation, env_, candidates);
  return New<LuaTranslation>(lua_, f);
}

LuaFilter::~LuaFilter() { VOID_CALL(fini_, "fini"); }

//--- LuaTranslator
LuaTranslator::LuaTranslator(const Ticket& ticket, Lua* lua)
  : Translator(ticket), lua_(lua) {
  an<LuaObj> init;
  InitTab luaobjs[]= { DEF_FIELD() };
  size_t size = sizeof(luaobjs) / sizeof(InitTab);

  lua->to_state([&](lua_State *L) {raw_init(L, ticket, &env_, luaobjs, size);});
  VOID_CALL(init, "init");
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

LuaTranslator::~LuaTranslator() { VOID_CALL( fini_, "fini"); }

//--- LuaSegmentor
LuaSegmentor::LuaSegmentor(const Ticket& ticket, Lua *lua)
  : Segmentor(ticket), lua_(lua) {
  an<LuaObj> init;
  InitTab luaobjs[]= { DEF_FIELD(),};
  size_t size = sizeof(luaobjs) / sizeof(InitTab);

  lua->to_state([&](lua_State *L) {raw_init(L, ticket, &env_, luaobjs, size);});
  VOID_CALL(init, "init");
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

LuaSegmentor::~LuaSegmentor() { VOID_CALL( fini_, "fini"); }


//--- LuaProcessor
LuaProcessor::LuaProcessor(const Ticket& ticket, Lua* lua)
  : Processor(ticket), lua_(lua) {
  an<LuaObj> init;
  InitTab luaobjs[]= { DEF_FIELD() };
  size_t size = sizeof(luaobjs) / sizeof(InitTab);

  lua->to_state([&](lua_State *L) {raw_init(L, ticket, &env_, luaobjs, size);});
  VOID_CALL(init, "init");
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

LuaProcessor::~LuaProcessor() { VOID_CALL( fini_, "fini"); }

}  // namespace rime
