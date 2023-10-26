#! /usr/bin/env lua
--
-- table_translator.lua
-- Copyright (C) 2023 Shewer Lu <shewer@gmail.com>
--
-- Distributed under terms of the MIT license.
--
--[[
--
''' custom.yaml'
patch:
  engine/translators/+:
    - lua_translator@*table_translator@translator
    - lua_translator@*script_translator@cangjie
````

------- methods
env.tran:finish_session  bool function()
env.tran:start_session   bool function()
env.tran:discard_session bool function()
env.tran:query           translation function(inp, seg)
env.tran:memorize        bool function(commit_entrys)
env.tran:update_entry    bool function(entry)

------- vars_set                   設定值
env.tran.max_homophones         =  number
env.tran.spelling_hints         =  number
env.tran.enable_correction      =  boolean
env.tran.memorize_callback      =  function(self, commits)
env.tran.enable_completion      =  false   boolean
env.tran.delimiters             =  false   string
env.tran.strict_spelling        =  boolean
env.tran.contextual_suggestions =  boolean
env.tran.initial_quality        =  double
env.tran.always_show_comments   =  boolean
env.tran.tag                    =  string

------- vars_get                     取值 
res = env.tran.max_homophones           1       number
res = env.tran.spelling_hints           0       number
res = env.tran.enable_correction        false   boolean
res = env.tran.memorize_callback        function: 0x562c732947e0        function
res = env.tran.enable_completion        true    boolean
res = env.tran.delimiters       '       string
res = env.tran.strict_spelling false    boolean
res = env.tran.contextual_suggestions   false   boolean
res = env.tran.initial_quality 0.0      number
res = env.tran.always_show_comments     false   boolean
res = env.tran.tag                      cangjie string


--]]

local M={}
local function simple_callback(self, commits)
  local context = self.engine.context
  if true then
    return self:memorize(commits)
  end
end
local function callback(self, commits) -- self : env.tran commits : list  
  local context = self.engine.context
  for i, entry in ipairs(commits:get()) do
		self:update_entry(entry, 0,"") -- do nothing to userdict
		-- self:update_entry(entry,1,"") -- update entry to userdict
		-- self:update_entry(entry,-1,"") -- delete entry to userdict
  end
end
function M.init(env)
  env.tran = Component.TableTranslator(env.engine, env.name_space, "table")
  env.tran.memorize_callback = simple_callback
end

function M.fini(env)
end

function M.func(inp, seg, env)
  local t = env.tran:query(inp,seg)
  if not t then return end
  for cand in t:iter() do
    yield(cand)
  end
end

return M
