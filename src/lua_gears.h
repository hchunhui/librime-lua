#ifndef RIME_LUA_GEARS_H_
#define RIME_LUA_GEARS_H_

#include <rime/translation.h>
#include <rime/filter.h>
#include <rime/translator.h>
#include <rime/segmentor.h>
#include <rime/processor.h>
#include <rime/gear/filter_commons.h>
#include "lib/lua.h"
#include <filesystem>
#include <ctime>

namespace fs = std::filesystem;
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

class LuaGearImpl {
public:
  LuaGearImpl(const Ticket& ticket, Lua* lua, const std::string& impl_name);
  ~LuaGearImpl();
  void ReloadIfModified();

protected:
  Lua *lua_;
  an<LuaObj> env_;
  an<LuaObj> func_;
  an<LuaObj> fini_;
  an<LuaObj> tags_match_;

  Ticket ticket_;
  std::string file_path_;
  std::string impl_name_;
  std::filesystem::file_time_type last_write_time_;
  std::time_t last_check_time_ = 0;
};

template <typename T>
class LuaGear : public LuaGearImpl {
public:
  LuaGear(const Ticket& ticket, Lua* lua)
    : LuaGearImpl(ticket, lua, typeid(T).name()) {}
};

class LuaFilter : public Filter, TagMatching, public LuaGear<LuaFilter> {
public:
  explicit LuaFilter(const Ticket& ticket, Lua* lua);

  virtual an<Translation> Apply(an<Translation> translation,
                                CandidateList* candidates);

  virtual bool AppliesToSegment(Segment* segment) {
    ReloadIfModified();
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
};

class LuaTranslator : public Translator, public LuaGear<LuaTranslator> {
public:
  explicit LuaTranslator(const Ticket& ticket, Lua* lua);

  virtual an<Translation> Query(const string& input,
                                const Segment& segment);
};

class LuaSegmentor : public Segmentor, public LuaGear<LuaSegmentor> {
public:
  explicit LuaSegmentor(const Ticket& ticket, Lua *lua);

  virtual bool Proceed(Segmentation* Segmentation);
};

class LuaProcessor : public Processor, public LuaGear<LuaProcessor> {
public:
  LuaProcessor(const Ticket& ticket, Lua *lua);

  virtual ProcessResult ProcessKeyEvent(const KeyEvent& key_event);
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
