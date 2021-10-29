/*
 * lua_memory.h
 * Copyright (C) 2021 Shewer Lu <shewer@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef LUA_MEMORY_H
#define LUA_MEMORY_H

#include <rime/dict/dictionary.h>
#include <rime/dict/user_dictionary.h>
#include <rime/gear/memory.h>
#include "lib/lua_templates.h"

using namespace rime;

namespace MemoryReg {

  class LuaMemory : public Memory {
    protected:
      an<LuaObj> memorize_callback;
      Lua *lua_;
      DictEntryIterator iter_;
      UserDictEntryIterator uter_;
    public:
      LuaMemory(Lua *lua, const Ticket& ticket)
        : lua_(lua), Memory(ticket) {}

      virtual bool Memorize(const CommitEntry&);
      void memorize(an<LuaObj> func) { memorize_callback = func; } 
      bool dictLookup( const string& input, const bool isExpand,size_t limit);
      bool userLookup( const string& input, const bool isExpand);
      optional<an<DictEntry>> dictNext();
      optional<an<DictEntry>> userNext();
      bool updateToUserdict( const DictEntry& entry, const int commits, const string& new_entry_prefix);
  };
}
#endif /* !LUA_MEMORY_H */
