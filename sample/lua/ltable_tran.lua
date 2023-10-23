#! /usr/bin/env lua
--
-- ltable_tran.lua
-- Copyright (C) 2023 Shewer Lu <shewer@gmail.com>
--
-- Distributed under terms of the MIT license.
-- callback  return bool

--  簡單 callback ,
local function callback(table_tran, commit_entry)
  if table_tran.engine.context:get_option("update_user_dict") then
    return table_tran:memorize(commit_entry) -- delegate super::Memorize()
  end
  return false
end
-- 
local function callback2(tran,commits)
  if table_tran.engine.context:get_option("update_user_dict") then 
    for i, dictentry in ipairs(commits:get()) do
      if (true) then
        tran:update_userdict(dictentry, 1, "") -- do update  user_dict  prefix_str = ""
      elseif (true) then
        tran:update_userdict(dictentry, -1, "") -- do delete user_dict prefix_str = ""
      else
        tran:update_userdict(dictentry, 0, "") -- do nothing  prefix_str = ""
      end
    end
    return true
  end
  return false
end



local M = {}
function M.init(env)
  env.tran = Component.TableTranslator(env.engine, Schema('cangjie5'), "translator", "")
  env.tran.memorize_callback = callback
  env.engine.context.option_update_notifier:connect(
  function (ctx,name)
    if name == "completion" then
      env.tran.enable_completion =  ctx:get_option(name) -- completion
    elseif name == "disable_userdict" then
      env.tran.disable_userdict = ctx:get_option(name) -- Query 屏蔽 user_dict 
    end
  end)
end
function M.fini(env)
end
function M.func(inp, seg, env)
  local translation = env.tran:query(inp,seg)
  if translatation then
    for cand in translation:iter() do
      yield(cand)
    end
  end
end

return M
