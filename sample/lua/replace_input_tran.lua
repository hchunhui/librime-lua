#! /usr/bin/env lua
--
-- replace_input_tran.lua
-- Copyright (C) 2023 Shewer Lu <shewer@gmail.com>
--
-- Distributed under terms of the MIT license.
-- 增加 lua_translator  query(input,seg.env)
-- 如果有 query() 將忽略 func()
-- query 是以操作 translation  如果不須要處理 cand 可以不經過 LuaTranslation
--
-- query(input,seg,env) return translation

local T={}

function T.init(env)
  local cl=ConfigList()
  -- 鍵盤置換
  cs:append( ConfigValue("xlit|dmatwfyzljxiekbhsocrugqnpv|abcdefghijklmnopqrstuvwxyz|").element)
  env.proj=Projection()
  env.proj:load(cl)
  env.keys='abcdefghijklmnopqrstuvwxyz'
  env.tran= Component.Translator(env.engine,'translator', 'table_translator') -- table_translator@translator
end
function T.fini(env)
end

-- 此模組因有 query() 轉而呼叫 query ,忽略 func()
function T.func(input,seg,env)
  local inp = env.proj:apply(input)
  for cand in env.tran(inp,seg):iter() do
    yield(cand)
  end
end

-- example 0 使用 LuaTranslation 產生 translation
function T.query0(input,seg, env)
  return Translation(T.func, input,seg,env) -- LuaTranslation(func ,...)
end

-- example 1 等同 example2
-- example 1 直接調用 table_translator
function T.query1(input,seg,env)
  local inp= env.proj:apply(input)  -- replace code
  return env.tran:query(inp,seg)  -- use table_translator
end

function T.query2(input,seg,env)
  if input:sub(#input)=='y' then  --   /y$/   then replace env.keys  a-z
    local tran = MergedTranslation()
    local inp = input:sub(1,#input-1)
    for char in env.keys:gmatch("[a-z]") do
      tran:append( env.tran:query(inp .. char, seg)) -- append translation  /y$/[a-z]$/
    end
    return tran
  else
    return env.tran:query(input,seg)
  end
end
--T.query = nil     -- lua_translator
--T.query = T.query0  -- lua_translator
--T.query = T.query1 --  table_translator
T.query = T.query2  -- 使用merge 有排序機制

return T
