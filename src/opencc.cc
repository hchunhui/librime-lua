// Copyright RIME Developers 
// Distributed under the BSD License
// from: <librime/src/rime/gear/simplifier.cc>
// 2011-12-12 GONG Chen <chen.sst@gmail.com>
// 


#include <opencc/Exception.hpp>
#include <opencc/Config.hpp> // Place OpenCC #includes here to avoid VS2015 compilation errors
#include <opencc/Converter.hpp>
#include <opencc/Conversion.hpp>
#include <opencc/ConversionChain.hpp>
#include <opencc/Dict.hpp>
#include <opencc/DictEntry.hpp>
#include <opencc/Common.hpp>
#include <boost/filesystem.hpp>
#include <rime/common.h>
#include <rime_api.h>

#include "lib/lua_export_type.h"
#include "lib/luatype_boost_optional.h"

using namespace std;
using boost::optional;

namespace {

class Opencc {
public:
  //static shared_ptr<Opencc> create(const string &config_path);
  Opencc(const string& config_path);
  bool ConvertWord(const string& text, vector<string>* forms);
  bool RandomConvertText(const string& text, string* simplified);
  bool ConvertText(const string& text, string* simplified);
  // --- lua wrap
  vector<string> convert_word( const string& text);
  string random_convert_text( const string& text);
  string convert_text( const string& text);

private:
  opencc::ConverterPtr converter_;
  opencc::DictPtr dict_;
};

/*
shared_ptr<Opencc> Opencc::create(const string &config_path) {
  try {
    return make_shared<Opencc>(config_path);
  }
  catch (opencc::FileNotFound &ex) {
    LOG(ERROR) << config_path << " : onpecc file not found";// << ex.what();
  }
  catch (opencc::InvalidFormat &ex) {
    LOG(ERROR) << config_path << " : opencc file InvalidFormat";// << ex.what();
  }
  catch (...){
    LOG(ERROR) << config_path << "Opencc ininialize faild" ;
  }
  return {};
}
*/
Opencc::Opencc(const string& config_path) {
  opencc::Config config;
  converter_ = config.NewFromFile(config_path);
  const list<opencc::ConversionPtr> conversions =
    converter_->GetConversionChain()->GetConversions();
  dict_ = conversions.front()->GetDict();
}


bool Opencc::ConvertText(const string& text, string* simplified) {
  if (converter_ == nullptr) return false;
  *simplified = converter_->Convert(text);
  return *simplified != text;
}

bool Opencc::ConvertWord(const string& text, vector<string>* forms) {
  if (dict_ == nullptr) return false;
  opencc::Optional<const opencc::DictEntry*> item = dict_->Match(text);
  if (item.IsNull()) {
    // Match not found
    return false;
  } else {
    const opencc::DictEntry* entry = item.Get();
    for (auto&& value : entry->Values()) {
      forms->push_back(std::move(value));
    }
    return forms->size() > 0;
  }
}

bool Opencc::RandomConvertText(const string& text, string* simplified) {
  if (dict_ == nullptr) return false;
  const char *phrase = text.c_str();
  std::ostringstream buffer;
  for (const char* pstr = phrase; *pstr != '\0';) {
    opencc::Optional<const opencc::DictEntry*> matched = dict_->MatchPrefix(pstr);
    size_t matchedLength;
    if (matched.IsNull()) {
      matchedLength = opencc::UTF8Util::NextCharLength(pstr);
      buffer << opencc::UTF8Util::FromSubstr(pstr, matchedLength);
    } else {
      matchedLength = matched.Get()->KeyLength();
      size_t i = rand() % (matched.Get()->NumValues());
      buffer << matched.Get()->Values().at(i);
    }
    pstr += matchedLength;
  }
  *simplified = buffer.str();
  return *simplified != text;
}
// for lua
string Opencc::convert_text(const string& text) {
  string res;
  return (ConvertText(text, &res)) ? res : text;
};

string Opencc::random_convert_text(const string& text) {
  string res;
  return (RandomConvertText(text, &res))? res : text ;
};

vector<string> Opencc::convert_word(const string& text){
  vector<string> res;
  if (ConvertWord(text, &res)){
    return res;
  }
  return {};
};

namespace OpenccReg {
  typedef Opencc T;
  namespace ns = boost::filesystem;

  optional<T> make(const string &filename) {
    string user_path( RimeGetUserDataDir());
    string shared_path(RimeGetSharedDataDir());
    user_path += "/opencc/" + filename;
    shared_path += "/opencc/" + filename;
    const string *path;
    if (ns::exists(user_path))
      path = &user_path;
    else if (ns::exists(shared_path))
      path = &shared_path;
    else
      path = &filename;

    try{
      return T(*path);
    }
    catch(...) {
      LOG(ERROR) << *path  << " File not found or InvalidFormat";
      return {};
    }
  }
  optional<vector<string>> convert_word(T &t,const string &s) {
    vector<string> res;
    if (t.ConvertWord(s,&res))
      return res;
    return {};
  }

  static const luaL_Reg funcs[] = {
    {"Opencc",WRAP(make)},
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    {"convert_word", WRAP(convert_word)},
    {"random_convert_text", WRAPMEM(T,random_convert_text)},
    {"convert_text", WRAPMEM(T,convert_text)},
    {"convert", WRAPMEM(T,convert_text)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };
}

}

void LUAWRAPPER_LOCAL opencc_init(lua_State *L) {
  EXPORT_TYPE(OpenccReg, L);
}
