#ifndef RIME_LUA_GEARS_H_
#define RIME_LUA_GEARS_H_

#include <rime/translation.h>
#include <rime/filter.h>
#include <rime/translator.h>
#include <rime/segmentor.h>
#include <rime/processor.h>
#include <rime/gear/filter_commons.h>
#include "lib/lua.h"

namespace rime {

class LuaTranslation : public Translation {
public:
  LuaTranslation(Lua *lua, an<LuaObj> f)
    : lua_(lua), f_(f) {
    Next();
  }

  bool Next();

  an<Candidate> Peek() {
    return c_;
  }

  virtual ~LuaTranslation();

private:
  Lua *lua_;
  an<Candidate> c_;
  an<LuaObj> f_;
};

class LuaFilter : public Filter, TagMatching {
public:
  explicit LuaFilter(const Ticket& ticket, an<Lua> lua);
  virtual ~LuaFilter();

  virtual an<Translation> Apply(an<Translation> translation,
                                CandidateList* candidates);

  virtual bool AppliesToSegment(Segment* segment) {
    if ( ! tags_match_ )
      return TagsMatch(segment);

    auto r = lua_->call<bool, an<LuaObj>, Segment *, an<LuaObj>>(tags_match_, segment,  env_);
    if (!r.ok()) {
      auto e = r.get_err();
      LOG(ERROR) << "LuaFilter::AppliesToSegment of " << name_space_ << " error(" << e.status << "): " << e.e;
      return false;
    }
    else
      return  r.get();
  }

private:
  an<Lua> lua_;
  an<LuaObj> env_;
  an<LuaObj> func_;
  an<LuaObj> fini_;
  an<LuaObj> tags_match_;
};

class LuaTranslator : public Translator {
public:
  explicit LuaTranslator(const Ticket& ticket, an<Lua> lua);
  virtual ~LuaTranslator();

  virtual an<Translation> Query(const string& input,
                                const Segment& segment);

private:
  an<Lua> lua_;
  an<LuaObj> env_;
  an<LuaObj> func_;
  an<LuaObj> fini_;
};

class LuaSegmentor : public Segmentor {
public:
  explicit LuaSegmentor(const Ticket& ticket, an<Lua> lua);
  virtual ~LuaSegmentor();

  virtual bool Proceed(Segmentation* Segmentation);

private:
  an<Lua> lua_;
  an<LuaObj> env_;
  an<LuaObj> func_;
  an<LuaObj> fini_;
};

class LuaProcessor : public Processor {
public:
  LuaProcessor(const Ticket& ticket, an<Lua> lua);
  virtual ~LuaProcessor();

  virtual ProcessResult ProcessKeyEvent(const KeyEvent& key_event);

private:
  an<Lua> lua_;
  an<LuaObj> env_;
  an<LuaObj> func_;
  an<LuaObj> fini_;
};

struct LuaSingletonFactory {
  LuaSingletonFactory() = default;
  ~LuaSingletonFactory() = default;
  an<Lua> instance();
private:
  std::weak_ptr<Lua> lua_;
};

template<typename T>
class LuaComponent : public T::Component {
private:
  an<LuaSingletonFactory> lua_factory_;

public:
  LuaComponent(an<LuaSingletonFactory> lua_factory) : lua_factory_(lua_factory) {};
  T* Create(const Ticket &a) {
    Ticket t(a.engine, a.name_space, a.name_space);
    return new T(t, lua_factory_->instance());
  }
};

} // namespace rime

#endif /* RIME_LUA_GEARS_H_ */
