#ifndef RIME_LUA_GEARS_H_
#define RIME_LUA_GEARS_H_

#include <rime/filter.h>
#include <rime/translator.h>
#include <rime/segmentor.h>
#include <rime/processor.h>
#include <rime/gear/filter_commons.h>
#include "lib/lua.h"

namespace rime {

class LuaFilter : public Filter, TagMatching {
public:
  explicit LuaFilter(const Ticket& ticket, Lua* lua);

  virtual an<Translation> Apply(an<Translation> translation,
                                CandidateList* candidates);

  virtual bool AppliesToSegment(Segment* segment) {
    return TagsMatch(segment);
  }
private:
  Lua *lua_;
  an<LuaObj> env_;
  an<LuaObj> func_;
};

class LuaTranslator : public Translator {
public:
  explicit LuaTranslator(const Ticket& ticket, Lua* lua);

  virtual an<Translation> Query(const string& input,
                                const Segment& segment);

private:
  Lua *lua_;
  an<LuaObj> env_;
  an<LuaObj> func_;
};

class LuaSegmentor : public Segmentor {
public:
  explicit LuaSegmentor(const Ticket& ticket, Lua *lua);

  virtual bool Proceed(Segmentation* Segmentation);

private:
  Lua *lua_;
  an<LuaObj> env_;
  an<LuaObj> func_;
};

class LuaProcessor : public Processor {
public:
  LuaProcessor(const Ticket& ticket, Lua *lua);

  virtual ProcessResult ProcessKeyEvent(const KeyEvent& key_event);

private:
  Lua *lua_;
  an<LuaObj> env_;
  an<LuaObj> func_;
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
