#! /usr/bin/env lua
--
-- rime.lua
-- Copyright (C) 2022 Shewer Lu <shewer@gmail.com>
--
-- Distributed under terms of the MIT license.
--

local P={}
local S={}
local T={}
local F={}
local UF={}


-- OK
function P.init(env)
  print( "** init Processor",env.name_space )
  env.ncomps={
    "ascii_composer",
    "recognizer",
    "key_binder",
    "speller",
    "punctuator",
    "selector",
    "navigator",
  }
  env.comps={}
  for i,v in ipairs(env.ncomps) do
    local comp=Component.Processor(env.engine,"",v)
    assert(comp,"failed : create processor of " .. v)
    table.insert(env.comps, Component.Processor(env.engine,"",v))
  end
  -- option: editor_mode  change editor_mode 
  env.neditors={
    "express_editor",
    "fluid_editor",
  }
  local function gen_editor(env)
    local mode = env.engine.context:get_option("editor_mode") and 2 or 1
    print("** " ,env.neditors[mode])
    env.editor= Component.Processor(env.engine,"",env.neditors[mode]) 
  end
  gen_editor(env)

  env.notifier = env.engine.context.option_update_notifier:connect(
  function(ctx,name)
  if name =="editor_mode" then
      gen_editor(env)
    end
  end)
end
function P.fini(env)
  env.notifier:disconnect()
end
function P.func(key,env)
  -- test  an<KeyEvent> 
  print( "-- processor func" .. env.name_space)
  local context = env.engine.context
  local key = KeyEvent(key:repr() )
  local Rejected,Accepted,Noop= 0,1,2
  for i,v in ipairs(env.comps) do
    res = v:process_key_event(key)
    if res == Rejected or res == Accepted then
      print( "--- processor index ",i,res,env.ncomps[i])
      return res
    end
  end
  --local editor_mode= context:get_option("editor_mode") and 2 or 1
  --print("--- processor editor mode ",env.neditors[editor_mode])
  return env.editor:process_key_event(key)
end
-- ok
function S.init(env)
  print( "** init segmentor",env.name_space )
  env.ncomps = {
    "ascii_segmentor",
    "matcher",
    "abc_segmentor",
    "punct_segmentor",
    "fallback_segmentor",
  }
  env.comps ={}

  for i,v in ipairs(env.ncomps) do
    local comp=Component.Segmentor(env.engine,"",v)
    assert(comp,"failed : create segmentor of " .. v)
    table.insert(env.comps,comp)
  end
end
function S.fini(env)
end
function S.func(segs,env)
  print( "-- segmentor func" .. env.name_space)
  local res=true
  for i,v in ipairs(env.comps) do
    res = v:proceed(segs)
    if not res then 
      print("segment break ---> ", i,env.ncomps[i],res)
      break
    end
  end
  print("segment break ---> ", res)
  return res
end

-- 只可以iter() 一個 translation 
function T.init(env)
  print( "** init translator",env.name_space )
  env.tran1 = Component.Translator(env.engine,"","table_translator@translator")
  assert(env.tran1, "failed: tran1 of table_translator ")

  local luna= Schema("luna_pinyin")
  luna.config:set_string("translator/initial_quality", "1.5")
  env.tran2 = Component.Translator(env.engine,luna,"translator","script_translator")
  assert(env.tran1, "failed: tran2 of stript_translator from luna_pinyin.schema")
  
end
function T.fini(env)
end
local function warp_tran(tran,input,seg)
  return Translation(function()
    for cand in tran:query(input,seg):iter() do
      yield(cand)
    end
  end)
end
local function warp_test(input,seg,env)
  return Translation(function()
    local l={"一","二","三","四"}
  print( "----- sub tran0 - 1")
    for i,txt in ipairs(l) do
      print(txt)
      local cand=Candidate("test", seg.start, seg._end,txt,"test")
      cand.quality=1.1
      yield(cand)
    end
  print( "----- sub tran0 - 2")
  for cand in warp_tran(env.tran1,input,seg):iter() do
  --:query(input,seg):iter() do 
    yield(cand)
  end
  print("---- sub tran0 - 3")
  for cand in env.tran2:query(input,seg):iter() do
    yield(cand)
  end
  end)
end

function T.func(input,seg,env)
  print( "-- translator func" .. env.name_space)
  --print("--- tran0 test" )
  --for cand in warp_test(input,seg,env):iter() do
    --yield(cand)
  --end
  print("--- tran1")
  for cand in warp_tran(env.tran1,input,seg):iter() do
  --:query(input,seg):iter() do 
    yield(cand)
  end
  print("--- tran2")
  for cand in env.tran2:query(input,seg):iter() do
    yield(cand)
  end

end

-- 不可加入 uniquifier
function F.init(env)
  print( "** init filter",env.name_space )
  env.ncomps={
    "simplifier",
    "single_char_filter",
  }
  env.comps={}
  for i,v in ipairs(env.ncomps) do
    local comp= Component.Filter(env.engine, "" , v)
    assert( comp , "failed : create filter comp: " .. v)
    table.insert(env.comps, comp)
  end
  env.reversedb= ReverseLookup("luna_pinyin")
  assert(env.reversedb, "failed create reversedb : luna_pinyin")
  assert(env.reversedb:lookup("百")=="bai bo", "luna_pinyin reverse lookup  not match")
end

function F.fini(env)
end

function F.tags_match(seg,env)
  print("- tags_match ", env.name_space)
  env.applyed={}
  env.napplyed={}
  for i,v in ipairs(env.comps) do
    if v:applies_to_segment(seg) then
      print("--  match tag filter ",i,env.ncomps[i])
      table.insert(env.napplyed,env.ncomps[i])
      table.insert(env.applyed,v)
    end
  end
  return true
end

function F.func(inp,env)
  print( "-- filter func" .. env.name_space)
  for i,v in ipairs(env.applyed) do
    print("----subfilter ",i)
    inp = v:apply(inp)
  end
  for cand in inp:iter() do 
    local code = env.reversedb:lookup(cand.text)
    local comment = cand.comment ..  "-" .. code .. "-" .. cand.quality
    local c = cand:get_genuine()
    c.comment = comment
    yield(cand)
  end
end




-- lua_filter@uniquifier
-- uniquifier 沒有繼承 TagMatch 
-- 無法使用 系統中斷 segmentation fault (core dumped)  ./rime_api_console
function UF.init(env)
  print( "** init filter",env.name_space )
  env.tags = Set{"abc",'cangjit5'}
  env.comp = Component.Filter(env.engine, "" , "uniquifier")
  assert(env.comp,"failed: create uniquifier ")
end
function UF.fini(env)
end
function UF.tags_match(seg,env)
  return seg.tags * env.tags and true or false
end

function UF.func(inp,env)
  print( "-- processor func" .. env.name_space)
  for cand in env.comp:apply(inp):iter() do 
    yield(cand)
  end
end

init_processor=P
init_segmentor=S
m_translator=T
init_filter=F
--uniquifier=UF
