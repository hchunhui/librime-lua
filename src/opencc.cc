/*
 * opencc.h
 * Copyright (C) 2022 Shewer Lu <shewer@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#include <opencc/Exception.hpp>
#include <opencc/Config.hpp> // Place OpenCC #includes here to avoid VS2015 compilation errors
#include <opencc/Converter.hpp>
#include <opencc/Conversion.hpp>
#include <opencc/ConversionChain.hpp>
#include <opencc/Dict.hpp>
#include <opencc/DictEntry.hpp>
#include "opencc.h"

using namespace OpenccReg;
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

