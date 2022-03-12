#! /usr/bin/env lua
--
-- _rescue.lua
-- Copyright (C) 2022 Shewer Lu <shewer@gmail.com>
--
-- Distributed under terms of the MIT license.
--
local M={}
local E={}
--
function E:set(env,comp)
  local id= env.engine.schema.schema_id
  self[id] = self[id] or {}
  self[id][comp .. env.name_space ]=true
end
function E:get(env)
  local id= env.engine.schema.schema_id
  local tab={}
  for k,v in next , self[id] or {} do
    table.insert(tab,  k  )
  end
  return table.concat( tab ,",")
end

function M.processor(key,env)
  E:set(env,"P@")
  return 2 -- Noop
end

function M.segmentor(seg,env)
  E:set(env,"S@")
  return true
end

function M.translator(input,seg,env)
  E:set(env,"T@")
  yield(Candidate("LuaError",seg.start,seg._end, "", "Err:" .. E:get(env) ))
end

function M.filter(input, env)
  E:set(env,"F@")
  for cand in input:iter() do
    if not cand.comment:match("- Err:") then
      cand.comment = cand.comment .. "- Err:" .. E:get(env)
    end
    yield(cand)
  end
end

Rescue_processor=M.processor
Rescue_segmentor=M.segmentor
Rescue_translator=M.translator
Rescue_filter=M.filter

--return Rescue
