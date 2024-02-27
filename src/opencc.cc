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
#include <rime/common.h>
#include <rime/service.h>

#include "lib/lua_export_type.h"
#include "optional.h"

using std::string;
using std::vector;
using std::list;
using namespace rime;

namespace {

class Opencc {
public:
  //static shared_ptr<Opencc> create(const path &config_path);
  Opencc(const string& utf8_config_path);
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

Opencc::Opencc(const string& utf8_config_path) {
  opencc::Config config;
  // OpenCC accepts UTF-8 encoded path.
  converter_ = config.NewFromFile(utf8_config_path);
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
  if (converter_ == nullptr) return false;
  const list<opencc::ConversionPtr> conversions =
        converter_->GetConversionChain()->GetConversions();
  vector<string> original_words{text};
  bool matched = false;
  for (auto conversion : conversions) {
    opencc::DictPtr dict = conversion->GetDict();
    if (dict == nullptr) return false;
    set<string> word_set;
    vector<string> converted_words;
    for (const auto& original_word : original_words) {
      opencc::Optional<const opencc::DictEntry*> item =
          dict->Match(original_word);
      if (item.IsNull()) {
        // No exact match, but still need to convert partially matched
        std::ostringstream buffer;
        for (const char* wstr = original_word.c_str(); *wstr != '\0';) {
          opencc::Optional<const opencc::DictEntry*> matched =
              dict->MatchPrefix(wstr);
          size_t matched_length;
          if (matched.IsNull()) {
            matched_length = opencc::UTF8Util::NextCharLength(wstr);
            buffer << opencc::UTF8Util::FromSubstr(wstr, matched_length);
          } else {
            matched_length = matched.Get()->KeyLength();
            buffer << matched.Get()->GetDefault();
          }
          wstr += matched_length;
        }
        const string& converted_word = buffer.str();
        // Even if current dictionary doesn't convert the word
        // (converted_word == original_word), we still need to keep it for
        // subsequent dicts in the chain. e.g. s2t.json expands 里 to 里 and
        // 裏, then t2tw.json passes 里 as-is and converts 裏 to 裡.
        if (word_set.insert(converted_word).second) {
          converted_words.push_back(converted_word);
        }
        continue;
      }
      matched = true;
      const opencc::DictEntry* entry = item.Get();
      for (const auto& converted_word : entry->Values()) {
        if (word_set.insert(converted_word).second) {
          converted_words.push_back(converted_word);
        }
      }
    }
    original_words.swap(converted_words);
  }
  // No dictionary contains the word
  if (!matched) return false;
  *forms = std::move(original_words);
  return forms->size() > 0;
}

bool Opencc::RandomConvertText(const string& text, string* simplified) {
  if (dict_ == nullptr) return false;
  const list<opencc::ConversionPtr> conversions =
        converter_->GetConversionChain()->GetConversions();
  const char* phrase = text.c_str();
  for (auto conversion : conversions) {
    opencc::DictPtr dict = conversion->GetDict();
    if (dict == nullptr) return false;
    std::ostringstream buffer;
    for (const char* pstr = phrase; *pstr != '\0';) {
      opencc::Optional<const opencc::DictEntry*> matched =
          dict->MatchPrefix(pstr);
      size_t matched_length;
      if (matched.IsNull()) {
        matched_length = opencc::UTF8Util::NextCharLength(pstr);
        buffer << opencc::UTF8Util::FromSubstr(pstr, matched_length);
      } else {
        matched_length = matched.Get()->KeyLength();
        size_t i = rand() % (matched.Get()->NumValues());
        buffer << matched.Get()->Values().at(i);
      }
      pstr += matched_length;
    }
    *simplified = buffer.str();
    phrase = simplified->c_str();
  }
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
  using T = Opencc;

  template<typename> using void_t = void;

  template<typename U, typename = void>
  struct COMPAT {
    static optional<T> make(const string &filename) {
      auto user_path = string(rime_get_api()->get_user_data_dir());
      auto shared_path = string(rime_get_api()->get_shared_data_dir());
      try{
        return T(user_path + "/opencc/" + filename);
      }
      catch(...) {
        try{
          return T(shared_path + "/opencc/" + filename);
        }
        catch(...) {
          LOG(ERROR) << " [" << user_path << "|" << shared_path << "]/opencc/"
                     << filename  << ": File not found or InvalidFormat";
          return {};
        }
      }
    }
  };

  template<typename U>
  struct COMPAT<U, void_t<decltype(std::declval<U>().user_data_dir.string())>> {
    static optional<T> make(const string &filename) {
      U &deployer = Service::instance().deployer();
      auto user_path = deployer.user_data_dir;
      auto shared_path = deployer.shared_data_dir;
      try{
        return T((user_path / "opencc" / filename).u8string());
      }
      catch(...) {
        try{
          return T((shared_path / "opencc" / filename).u8string());
        }
        catch(...) {
          LOG(ERROR) << " [" << user_path << "|" << shared_path << "]/opencc/"
                     << filename  << ": File not found or InvalidFormat";
          return {};
        }
      }
    }
  };

  optional<vector<string>> convert_word(T &t,const string &s) {
    vector<string> res;
    if (t.ConvertWord(s,&res))
      return res;
    return {};
  }

  static const luaL_Reg funcs[] = {
    {"Opencc",WRAP(COMPAT<Deployer>::make)},
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
