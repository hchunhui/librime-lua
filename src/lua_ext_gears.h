/*
 * lua_ext_gears.h
 * Copyright (C) 2022 Shewer Lu <shewer@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef LUA_EXT_GEARS_H
#define LUA_EXT_GEARS_H
#include <rime/common.h>
#include <rime/ticket.h>
// LuaMemory
#include <rime/gear/memory.h>
#include <rime/dict/dictionary.h>
#include <rime/dict/user_dictionary.h>

namespace rime {
  class LuaMemory : public Memory {
    an<LuaObj> memorize_callback;
    Lua *lua_;
    public:
    using Memory::Memory;
    DictEntryIterator iter;
    UserDictEntryIterator uter;

    LuaMemory(Lua *lua, const Ticket& ticket)
      : lua_(lua), Memory(ticket) {}

    virtual bool Memorize(const CommitEntry&);

    void memorize(an<LuaObj> func) {
      memorize_callback = func;
    }

    void clearDict() {
      iter = DictEntryIterator();
    }
    void clearUser() {
      uter = UserDictEntryIterator();
    }
  };
}

#endif /* !LUA_EXT_GEARS_H */
