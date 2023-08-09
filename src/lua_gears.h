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

std::ostream& operator<<(std::ostream &os, const LuaErr &e);

class LuaTranslation : public Translation {
public:
  LuaTranslation(Lua *lua, an<LuaObj> f)
    : LuaTranslation(lua, f, "") {};
  LuaTranslation(Lua *lua, an<LuaObj> f, string name_space)
    : lua_(lua), f_(f), name_space_(name_space) {
    Next();
  }

  bool Next();

  an<Candidate> Peek() {
    return c_;
  }
  string name_space() const { return name_space_; }
  void set_name_space(const string& ns) { name_space_ = ns;}

private:
  Lua *lua_;
  an<Candidate> c_;
  an<LuaObj> f_;
  string name_space_;
};

class LuaFilter : public Filter, TagMatching {
public:
  explicit LuaFilter(const Ticket& ticket, Lua* lua);
  virtual ~LuaFilter();

  virtual an<Translation> Apply(an<Translation> translation,
                                CandidateList* candidates);

  virtual bool AppliesToSegment(Segment* segment);

private:
  Lua *lua_;
  an<LuaObj> env_;
  an<LuaObj> func_;
  an<LuaObj> fini_;
  an<LuaObj> tags_match_;
};

class LuaTranslator : public Translator {
public:
  explicit LuaTranslator(const Ticket& ticket, Lua* lua);
  virtual ~LuaTranslator();

  virtual an<Translation> Query(const string& input,
                                const Segment& segment);

private:
  Lua *lua_;
  an<LuaObj> env_;
  an<LuaObj> func_;
  an<LuaObj> fini_;
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
