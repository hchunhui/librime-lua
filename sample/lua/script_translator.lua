#! /usr/bin/env lua
--
-- script_translator.lua
-- Copyright (C) 2023 Shewer Lu <shewer@gmail.com>
--
-- Distributed under terms of the MIT license.
--

--[[

''' custom.yaml'
patch:
  engine/translators/+:
    - lua_translator@*table_translator@translator
    - lua_translator@*script_translator@cangjie
````
------- methods          return  
env.tran:start_session   false   function()
env.tran:finish_session  false   function()
env.tran:discard_session false   function()
env.tran:query   false   function(inp, seg)
env.tran:memorize        false   function(commit_entrys)
env.tran:update_entry    false   function(entry, state, prefix_str)

------- vars_set
env.tran.spelling_hints         =    int >0
env.tran.initial_quality        =    double
env.tran.contextual_suggestions =    boolean
env.tran.enable_completion      =    boolean
env.tran.always_show_comments   =    boolean
env.tran.strict_spelling        =    boolean
env.tran.max_homophones         =    int
env.tran.memorize_callback      =    function(self, commit_entry)
env.tran.enable_correction      =    boolean
env.tran.tag                    =    string
env.tran.delimiters             =    string

------- vars_get
res = env.tran.spelling_hints           0       number
res = env.tran.initial_quality          0.0     number
res = env.tran.contextual_suggestions   false   boolean
res = env.tran.enable_completion        true    boolean
res = env.tran.always_show_comments     false   boolean
res = env.tran.strict_spelling          false   boolean
res = env.tran.max_homophones           1       number
res = env.tran.memorize_callback        function: 0x557a0c15ef40        function
res = env.tran.enable_correction        false   boolean
res = env.tran.tag                      abc     string
res = env.tran.delimiters               '       string

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
		self:update_entry(entry,0,"") -- do nothing to userdict
		-- self:update_entry(entry,1,"") -- update entry to userdict
		-- self:update_entry(entry,-1,"") -- delete entry to userdict
  end
end
function M.init(env)
  env.tran = Component.ScriptTranslator(env.engine, env.name_space, "table")
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
