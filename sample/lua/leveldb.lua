#! /usr/bin/env lua
--
-- leveldb.lua
-- Copyright (C) 2022 Shewer Lu <shewer@gmail.com>
--
-- Distributed under terms of the MIT license.
--
-- 
--[[
leveldb 可以使用KEY 查詢 value 或是 query 查詢 frefix key (字元排序 而不是用trie)

ex ab< abc < ac < ad

-- rime.lua

user_tran=require 'leveldb'
-- schema.yaml
# append to engine/translations
lua_translator@user_tran@ecdict

ecdict:
  dictionary: ecdictdb
  initial_quality: 1.5

--]]

db_pool_={}
local function opendb(fn,dbname) 
  dbname= dbname or ""
  local db = db_pool_[fn]
  if not db then 
    db = LevelDb(fn,dbname) 
    if not db then return nil end
    db_pool_[fn] = db
  end
  if not db:loaded() then db:open() end
  return db
end
local function init_data(db)
  local tab = {
    ab = "于",
    cd = "金",
    ac = "金金",
  }
  for k,v in next,tab do
    db:update(k,v)
  end
end

local M={}
function M.init(env)
  local config = env.engine.schema.config
  local dbname = config:get_string(env.name_space .. "/dictionary")
  env.quality= tonumber( config:get_string(env.name_space .. "/initial_quality") ) or 1
  env.db = assert(opendb(dbname,'dict'), "leveldb cand not init")
  init_data(env.db)
end

function M.fini(env)
  env.db:close()
end

function M.func(inp,seg,env)
  for k,v in env.db:query(inp):iter() do 
    local type_ = k == inp and "udata" or "comp_udada"
    local cand = Candidate(type_,seg.start,seg._end,k,v)
    cand.quality= env.quality
    yield(cand)
  end
end

return M

