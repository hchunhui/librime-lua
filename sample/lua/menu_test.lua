#! /usr/bin/env lua
--
-- menu.lua
-- Copyright (C) 2023 Shewer Lu <shewer@gmail.com>
--
-- Distributed under terms of the MIT license.
--
-- Menu()   add method: menu:add_filter( filter)
--          add get:  menu.count 
local T={}
function T.init(env)
  env.tran = Component.Translatior(env.engine,"translator", "table_translator")
  env.filter= Component.Filter(env.engine, "filter", "simplifier")
end
function T.fini(env)
end




-- 方法1
function T.func(input,seg,env)
  local menu= Menu()
  menu:add_translation( env.tran:query(input,seg))
  menu:add_filter(filter)
  local index = 0
  while (true) do
    local cand = menu:get_candidate_at(index)
    if not cand then break end
     index = index + 1
    yield(cand)
  end
end
local function tran_menu(menu)
  local index=0
  while(true) do
    local cand = menu:get_candidate_at(index)
    if not cand then break end
    index = index + 1
    yield(cand)
  end
end
--  use LuaTranslation(func, ...)
function T.func2(input,seg,env)
  local menu= Menu()
  menu:add_translation( env.tran:query(input,seg))
  menu:add_filter(filter)
  for cand in Translation(tarn_menu, menu):iter() do 
    yield(cand)
  end
end

return T
