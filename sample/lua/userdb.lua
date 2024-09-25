#! /usr/bin/env lua
--
-- userdb.lua
-- Copyright (C) 2024 Shewer Lu <shewer@gmail.com>
--
-- Distributed under terms of the MIT license.
--
--[[
example:
local userdb = require 'userdb'
local ldb=userdb.LevelDb('ecdict')
ldb:open()
for k,v in ldb:query('a'):iter() do print(k,v) end

--]]
local db_pool_ = {}
local vars_get= {
  _loaded=true,
  read_only=true,
  disabled=true,
  name=true,
  file_name=true,
  }
local vars_set= {}
local userdb_mt = {}
function userdb_mt.__newindex(tab,key,value)
  local db = db_pool_[tab._db_key]
  if not db then
    db = UserDb(tab._db_name, tab._db_class)
    db_pool_[tab._db_key] = db
  end
  if db and vars_set[key] then
    db[key]= value
  end
end
  
function userdb_mt.__index(tab,key)
  local db = db_pool_[tab._db_key]
  if not db then
    db = UserDb(tab._db_name, tab._db_class)
    db_pool_[tab._db_key] = db
  end

  if db and vars_get[key] then
    return db[key]
  else
    return function (tab, ...)
      return db[key](db,...)
    end
  end
end

local userdb= {}

function userdb.UserDb(db_name, db_class)
  local db_key = db_name .. "." .. db_class
  local db = {
    _db_key = db_key,
    _db_name= db_name,
    _db_class = db_class,
  }
  db_pool_[db_key] = UserDb(db_name, db_class)
  return setmetatable(db , userdb_mt)
end

function userdb.LevelDb(db_name)
  return userdb.UserDb(db_name, "userdb")
end
function userdb.TableDb(db_name)
  return userdb.UserDb(db_name, "plain_userdb")
end
  
return userdb
