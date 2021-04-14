#! /usr/bin/env lua
--
-- table_translator.lua
-- Copyright (C) 2021 Shewer Lu <shewer@gmail.com>
--
-- Distributed under terms of the MIT license.
--



-- local p= Projection() --  new obj
-- p:load( ConfigList )  -- load from ConfigList 
-- p:append(string)      -- append from  string



local function test_code(env)
	print("==== test TranslatorOptions ")
	print("----tran: ",env.translator,env.translator.lookup)
	print("----tran: ",env.translator,env.translator.memory.dict)
--	print("----tran: ",env.translator,env.translator.memory.dict.lookup_words)
	print("tag: " ,env.translator.option.tag)
	print("enable_completion: " ,env.translator.option.enable_completion )
	env.translator.option.enable_completion=false
end 


local function init_func(env)
	--  create Ticket
	local ticket=Ticket(env.engine, "translator") -- (engine , path)
	assert(ticket, ticket) 
	print( "load TableTran : cagjie5" )
	-- create TableTranslator obj
	env.translator= TableTranslator( ticket ) 
	env.translator.option.tag="abc"
	env.translator.option.enable_completion=true
	test_code(env)--  
end 

local function func(input,seg,env)
    --   lookup(input, seg.start,seg_end,limit,enable_user_dict)
    local t1= env.translator:lookup(input,seg.start,seg._end,10,true)
	local preedit1= env.translator.option.preedit_formatter:apply(input) 
    local t2= env.translator:lookup("j".. input:sub(2,-1),seg.start,seg._end,10,true)

	local pj2= env.translator.option.preedit_formatter  -- get Projection obj
	local preedit2= pj2:apply("j" .. input:sub(2,-1) ) 

	local pj3= Projection()
	local configlist= env.engine.schema.config:get_list("translator/preedit_format") 
	pj3:load( configlist ) 
	local preedit3=pj3:apply(input) 
    -- merge translation into tab
	local tab={}
	for cand in t1:iter() do
			cand.comment= preedit1 .. cand.quality
			table.insert(tab,cand)
	end 
	for cand in t2:iter() do
			cand.comment= preedit2 ..":" .. preedit3.. ":" .. cand.quality
			table.insert(tab,cand)
	end 
    -- sort quality of cand 
	table.sort(tab,function(a,b) return a.quality > b.quality end )
	
	for _,cand in next, tab do
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

