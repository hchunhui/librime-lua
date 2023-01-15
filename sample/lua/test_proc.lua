#! /usr/bin/env lua
--
-- test_proc.lua
-- Copyright (C) 2023 Shewer Lu <shewer@gmail.com>
--
-- Distributed under terms of the MIT license.
--

local function _test(env,test_list,exit_,format)
  local lu = require 'tools/luaunit'
  _G['env'] = env
  format = format or 'tap'

  local lu=require 'tools/luaunit'
  lu.LuaUnit:runSuiteByInstances(test_list or {},format) 

  _G['env'] = nil
  if exit_ then 
    os.exit()
  end
end

print('-----pre_test without env.engine -----')
local pretest_list={
  {'test_list', require 'test/list_test'},
}
_test(nil, pretest_list)

local M={}
function M.init(env)
  print('-----test with env.engine -----')
  local test_list={
    {'test_translator', require 'test/test_translator'},
  }
  _test(env,test_list,true)
end

function M.fini(env)
end

function M.func(key,env)
  return 2
end

return M
