#! /usr/bin/env lua
--
-- table_translator.lua
-- Copyright (C) 2021 Shewer Lu <shewer@gmail.com>
--
-- Distributed under terms of the MIT license.
--
--

local function test_code(env)
  print("==== test TranslatorOptions ")
  print("tag: " ,env.translator.option.tag)
  print("enable_completion: " ,env.translator.option.enable_completion )
  env.translator.option.enable_completion=false
end

local function init_func(env)
  --  create Ticket
  local ticket=Ticket(env.engine, "cangjie5") -- (engine , path)
  assert(ticket, ticket)
  print( "load TableTran : cagjie5" )
  -- create TableTranslator obj
  env.translator= TableTranslator( ticket )
  test_code(env)--
  env.translator.option.tag="abc"
  env.translator.option.enable_completion=true
end

local function func(input,seg,env)
    -- query 返回的 translation  只能iter()一次
  local tran= env.translator:query(input,seg)

  for cand in tran:iter() do
    -- reprace cand.comment
    yield(cand)
  end
end


return { init=init_func, func=func }
---   rime.lua
--   table_tran= require("table_translator")()
--
--
--   schema.yaml
--   insert to /engine/translators
--   lua_translator@tab_tran
--

