
-- local p= Projection() --  new obj
-- p:load( ConfigList )  -- load from ConfigList 
-- p:append(string)      -- append from  string



local function test_code(env,pfl2,preedit_fmt_list)
	print("==== test get_int bool double string =====")
	local b=env.engine.schema.config:get_bool(  "translator/enable_encoder")
	local i=env.engine.schema.config:get_int(   "menu/page_size")
	local d=env.engine.schema.config:get_double("translator/initial_quality")
	local s=env.engine.schema.config:get_string("translator/tips")
	print( "bool", b , 
		   "int" , i ,
		   "double", d,
		   "string", s)

	-- print value 
	print(" ConfigList from translator/preedit")
	for i=0,preedit_fmt_list.size-1 do 
		print(i,  preedit_fmt_list:get_value_at(i).value)
	end 

	-- print value 
	print(" ConfigList from user")
	for i=0,pfl2.size-1 do 
		print(i,  preedit_fmt_list:get_value_at(i).value)
	end 
	--  test projection  :  p1   p2
	local str1= "abcdefg"
	print("porjection test :" , str1 , env.p1:apply(str1), env.p2:apply(str1) )
	--  
end 

local function init_func(env)
	-- load ConfigList  form  schema path
	local preedit_fmt_list = env.engine.schema.config:get_list("translator/preedit_format")
	--   create Projection obj
	local p1=Projection()
	p1:load(preedit_fmt_list)

	--  user create  Projection obj
	--  create ConfigList obj 
	local pfl2= ConfigList()
	-- create ConfigValue    &  ConfigList  ust isnert( index, ConfigValue)  append()
	local v1= ConfigValue("xlit|abc|xyz|")
	pfl2:append(v1.element)
	pfl2:append( ConfigValue("xlit|def|tuv|").element )  -- element referce to (ConfigItem *)  
	pfl2:insert(1,ConfigValue("xlit|ghi|qrs|").element )


	--   create Projection obj
	local p2=Projection()
	p2:load(pfl2)
	env.p1=p1
	env.p2=p2
    -- 
    test_code( env, pfl2, preedit_fmt_list )

end 
local function func(input,seg,env)
	for cand in input:iter() do 
		-- reprace cand.comment 
		cand.comment=  env.p1(cand.comment) .. env.p2(cand.comment)
		yield(cand)
	end
end


return { init=init_func, func=func } 
---   rime.lua    
--   projection_filter= require("projection.lua")
--
--
--   schema.yaml
--   insert to /engine/filter
--   lua_filter@projection_filter
--

