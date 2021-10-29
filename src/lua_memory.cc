/*
 * lua_memory.cc
 * Copyright (C) 2021 Shewer Lu <shewer@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "lua_memory.h"

namespace MemoryReg {

  bool LuaMemory::Memorize(const CommitEntry& commit_entry) {
    if (!memorize_callback)
      return false;

    auto r = lua_->call<bool, an<LuaObj>, const CommitEntry &>(memorize_callback, commit_entry);
    if (!r.ok()) {
      auto e = r.get_err();
      LOG(ERROR) << "LuaMemory::Memorize error(" << e.status << "): " << e.e;
      return false;
    } else
      return r.get();
  }

  bool LuaMemory::dictLookup(const string& input, const bool isExpand,size_t limit) {
    iter_ = DictEntryIterator();
    limit = limit == 0 ? 0xffffffffffffffff : limit;
    if (dict_)
      return dict_->LookupWords(&iter_, input, isExpand, limit) > 0;
    else
      return false;
  }

  optional<an<DictEntry>> LuaMemory::dictNext() {
    if (iter_.exhausted()) {
      return {};
    }
    an<DictEntry> ret = iter_.Peek();
    iter_.Next();
    return ret;
  }

  bool LuaMemory::userLookup(const string& input, const bool isExpand) {
    uter_ = UserDictEntryIterator();
    if (user_dict_)
      return user_dict_ ->LookupWords(&uter_, input, isExpand) > 0;
    else
      return false;
  }

  optional<an<DictEntry>> LuaMemory::userNext() {
    if (uter_.exhausted()) {
      return {};
    }
    an<DictEntry> ret = uter_.Peek();
    uter_.Next();
    return ret;
  }

  bool LuaMemory::updateToUserdict(const DictEntry& entry, const int commits, const string& new_entry_prefix) {
    if (user_dict_)
      return user_dict_ ->UpdateEntry(entry, commits, new_entry_prefix);
    else
      return false;
  }

}

