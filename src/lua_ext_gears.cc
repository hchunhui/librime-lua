/*
 * lua_ext_gears.cc
 * Copyright (C) 2022 Shewer Lu <shewer@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */
#include "lib/lua_templates.h"
#include "lua_ext_gears.h"


namespace rime {

  bool LuaMemory::Memorize(const CommitEntry& commit_entry ) {
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

}


