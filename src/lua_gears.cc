#include "lib/lua_templates.h"
#include "lua_gears.h"
#include <vector>

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

static std::vector<std::string> split_string(const std::string& str, const std::string& delimiter) {
    std::vector<std::string> result;
    size_t pos = 0;
    size_t found;
    while ((found = str.find(delimiter, pos)) != std::string::npos) {
        result.push_back(str.substr(pos, found - pos));
        pos = found + delimiter.length();
    }
    result.push_back(str.substr(pos));
    return result;
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

  std::string _sub_func = "";
  if (t.klass.size() > 1 && t.klass[0] == '*') {
    lua_getglobal(L, "require");
    std::vector<std::string> _vec_klass = split_string(t.klass.substr(1), "*");
    int _vec_klass_sz = _vec_klass.size();
    lua_pushstring(L, _vec_klass.at(0).c_str());
    int status = lua_pcall(L, 1, 1, 0); // call module file
    bool parse_ok = true;
    if (status == LUA_OK && _vec_klass_sz > 2) {
      for (auto i = 1; i < _vec_klass_sz - 1; i++) {
        if (lua_type(L, -1) == LUA_TTABLE) {
          lua_getfield(L, -1, _vec_klass.at(i).c_str());
          if (lua_type(L, -1) != LUA_TTABLE) {
            parse_ok = false;
            LOG(ERROR) << "Lua Compoment of initialize  error:("
              << " module: "<< _vec_klass.at(0)
              << ", name_space: " << t.name_space
              << ", sub-table \"" << _vec_klass.at(i) << "\" type: " << luaL_typename(L, -1)
              << " ): " << "type error expect table ";
            break;
          }
        }
      }
    }
    if (_vec_klass.size() >= 2 && status == LUA_OK && parse_ok)
      _sub_func = _vec_klass.at(_vec_klass_sz - 1);

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
repeat_check:
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

    if (_sub_func.empty()) {
      lua_getfield(L, -1, "func");
    } else {
      lua_getfield(L, -1, _sub_func.c_str());
      // if last key is table, repeat_check without _sub_func
      if (lua_type(L, -1) == LUA_TTABLE) {
        _sub_func.clear();
        goto repeat_check;
      }
    }
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

//--- LuaFilter
LuaFilter::LuaFilter(const Ticket& ticket, Lua* lua)
  : Filter(ticket), TagMatching(ticket), lua_(lua) {
  lua->to_state([&](lua_State *L) {raw_init(L, ticket, &env_, &func_, &fini_, &tags_match_);});
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
      LOG(ERROR) << "LuaTranslator::~LuaTranslator of "<< name_space_ << " error(" << e.status << "): " << e.e;
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
  lua->to_state([&](lua_State *L) {raw_init(L, ticket, &env_, &func_, &fini_);});
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
