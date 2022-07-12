#! /usr/bin/env lua
--
-- rime.lua
-- Copyright (C) 2022 Shewer Lu <shewer@gmail.com>
--
-- Distributed under terms of the MIT license.
--
--[[
Component.Processor(engine, [schema,] ns, <klass>@<ns>)
Component.Segmentor(engine, [schema,] ns, <klass>@<ns>)
Component.Translator(engine, [schema,] ns, <klass>@<ns>)
Component.Filter(env.engine, [schema,] ns, <klass>@<ns>)

ex:
-- return an<Translator> load config from  <self schema_id>.schema:/translator
Component.Translator(env.engine,"","script_translator")
-- return an<Translator> load config from  luna_pinyin.schema:/translator
Component.Translator(env.engine,Schema(luna_pinyin"),"","script_translator")

-- rime.lua
require 'component_text'

init_processor=P
init_segmentor=S
m_translator=T
init_filter=F

test.schema.yaml
engine:
  processors:
    - lua_processor@(init_processor
  segmentors:
    - lua_segmentor@init_segmentor
  translators:
    - lua_translator@m_translator
  filters:
    - lua_filter@init_filter

--]]
local P={}
local S={}
local T={}
local F={}

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
  local context = env.engine.context
  local key = KeyEvent(key:repr() )
  local Rejected,Accepted,Noop= 0,1,2
  for i,v in ipairs(env.comps) do
    res = v:process_key_event(key)
    if res == Rejected or res == Accepted then
      return res
    end
  end
  return env.editor:process_key_event(key)
end

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
  local res=true
  for i,v in ipairs(env.comps) do
    res = v:proceed(segs)
    if not res then
      break
    end
  end
  return res
end

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

function T.func(input,seg,env)
  local comp_cand1
  local t1= env.tran1:query(input,seg)
  for cand in t1:iter() do
    if cand.type == "compoletion" then
      ccomp_cand1 = cand
      break
    end
    yield(cand)
  end
  local comp_cand2
  local t2 = env.tran2:query(input,reg)
  for cand in t2:iter() do
    if cand.type == "compoletion" then
      ccomp_cand2 = cand
      break
    end
    yield(cand)
  end
  yield(comp_cand1)
  for cand in t1:iter() do
    yield(cand)
  end
  yield(comp_cand2)
  for cand in t2:iter() do
    yield(cand)
  end
end

function F.init(env)
  print( "** init filter",env.name_space )
  env.ncomps={
    "simplifier",
    "single_char_filter",
    "uniquifier"
  }
  env.comps={}
  for i,v in ipairs(env.ncomps) do
    local comp= Component.Filter(env.engine, "" , v)
    assert( comp , "failed : create filter comp: " .. v)
    table.insert(env.comps, comp)
  end
  -- reversedb test
  env.reversedb= ReverseLookup("luna_pinyin")
  assert(env.reversedb, "failed create reversedb : luna_pinyin")
  assert(env.reversedb:lookup("百")=="bai bo", "luna_pinyin reverse lookup  not match")
end

function F.fini(env)
end

function F.tags_match(seg,env)
  env.applyed={}
  env.napplyed={}
  for i,v in ipairs(env.comps) do
    if v:applies_to_segment(seg) then
      table.insert(env.napplyed,env.ncomps[i])
      table.insert(env.applyed,v)
    end
  end
  return true
end
--  new args  cands for subfilter:apply(inp,cands)
function F.func(inp,env,cands)
  for i,v in ipairs(env.applyed) do
    inp = v:apply(inp,cands)
  end
  for cand in inp:iter() do
    local code = env.reversedb:lookup(cand.text)
    local comment = cand.comment ..  "-" .. code .. "-" .. cand.quality
    local c = cand:get_genuine()
    c.comment = comment
    yield(cand)
  end
end

init_processor=P
init_segmentor=S
m_translator=T
init_filter=F
