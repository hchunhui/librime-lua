#! /usr/bin/env lua
--
-- _rescue.lua
-- Copyright (C) 2022 Shewer Lu <shewer@gmail.com>
--
-- Distributed under terms of the MIT license.
--
local M={}

M.err_components={}
local function comment_msg(err_comps)
  local tab={}
  for k,v in next,err_comps do 
    table.insert(tab, v and k or nil )
  end
  return #tab > 1 and table.concat(tab,",") or ""
end
function M.Rescue_processor(key,env)
  return 2 -- Noop
end

function M.segment(seg,env)
  return true
end
function M.translato(input,seg,env)
  yield(Candidate("LuaError",seg.start,seg._end, "", "Err:" .. comment_msg(M.err_components) ))
end

function M.filter(input, env)
  for cand in input:iter() do
    if not cand.comment:match("- Err:") then  
      cand.comment = cand.comment .. "- Err:" .. comment_msg(M.err_components)
    end 
    yield(cand)
  end
end
_Rescue=M


function Rescue(...)
  local arg,d1,d2= ...
  print("type arg",type(arg),type(d1),d1.name_space,type(d2))
  if type(arg) == "userdata" then
    if arg.shift then
      func =M.processor
      comp="P@" .. (d1 and d1.name_space or "p")
    elseif arg.tran then
      func= M.segment
      comp="S@" .. (d1 and d1.name_space or "s")
    elseif arg.iter then
      func= M.filter
      comp="F@" .. ( d1 and d1.name_space or "f")
    end
  elseif type(arg) == "string" then
    comp="T@" .. (d2 and d2.name_space or "t")
    func= M.translato
  end
  if comp then
    M.err_components[comp]=true
  end 
  return  func and  func(arg, ...)

end

return Rescue
