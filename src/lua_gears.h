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

private:
  Lua *lua_;
  an<Candidate> c_;
  an<LuaObj> f_;
};

class LuaFilter : public Filter, TagMatching {
public:
  explicit LuaFilter(const Ticket& ticket, Lua* lua);
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
  an<Translation> LuaApply(an<Translation>, CandidateList*);
  Lua *lua_;
  an<LuaObj> env_;
  an<LuaObj> func_;
  an<LuaObj> fini_;
  an<LuaObj> tags_match_;
  an<LuaObj> apply_;
};

class LuaTranslator : public Translator {
public:
  explicit LuaTranslator(const Ticket& ticket, Lua* lua);
  virtual ~LuaTranslator();

  virtual an<Translation> Query(const string& input,
                                const Segment& segment);

private:
  an<Translation> LuaQuery(const string&, const Segment&);
  Lua *lua_;
  an<LuaObj> env_;
  an<LuaObj> func_;
  an<LuaObj> fini_;
  an<LuaObj> query_;
};

class LuaSegmentor : public Segmentor {
public:
  explicit LuaSegmentor(const Ticket& ticket, Lua *lua);
  virtual ~LuaSegmentor();

  virtual bool Proceed(Segmentation* Segmentation);

private:
  Lua *lua_;
  an<LuaObj> env_;
  an<LuaObj> func_;
  an<LuaObj> fini_;
};

class LuaProcessor : public Processor {
public:
  LuaProcessor(const Ticket& ticket, Lua *lua);
  virtual ~LuaProcessor();

  virtual ProcessResult ProcessKeyEvent(const KeyEvent& key_event);

private:
  Lua *lua_;
  an<LuaObj> env_;
  an<LuaObj> func_;
  an<LuaObj> fini_;
};

template<typename T>

class LuaComponent : public T::Component {
private:
  an<Lua> lua_;

public:
  LuaComponent(an<Lua> lua) : lua_(lua) {};
  T* Create(const Ticket &a) {
    Ticket t(a.engine, a.name_space, a.name_space);
    return new T(t, lua_.get());
  }
};

} // namespace rime

#endif /* RIME_LUA_GEARS_H_ */
