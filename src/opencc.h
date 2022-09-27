/*
 * opencc.h
 * Copyright (C) 2022 Shewer Lu <shewer@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef _LUA_OPENCC_H
#define _LUA_OPENCC_H
#include <opencc/Common.hpp>
using namespace std;

namespace OpenccReg {

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
}

#endif /* !OPENCC_H */
