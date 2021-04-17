
local function init_func(env)

	local config=env.engine.schema.config
    -- load ConfigList  form  path
	local proedit_fmt_list = conifg:get_list("translastor/preedit_format")
	-- print value 
	for i=0,proedit_fmt_list.size do 
		print(i,  proedit_fmt_list:get_value_at(i).value)
	end 
	--   create Projection obj
	local p1=Projection()
	p1:load(proedit_fmt_list)
    
	--  user create  Projection obj
	local pfl2= ConfigList()
	-- create ConfigValue    &  ConfigList  ust isnert( index, ConfigValue)  append()
	local v1= ConfigValue("xlit|abc|xyz|")
	pfl2:append(v1.element)
	pfl2:append( ConfigValue("xlit|def|tuv|").element )  -- element referce to (ConfigItem *)  
	pfl2:insert(1,ConfigValue("xlit|ghi|qrs|").element )
	local p2=Projection()
	p2:load(pfl2)
	local str1= "abcdefg"
	--   
	print(str1 , p1:apply(str1), p2:apply(str1) )
	--  
	env.p1=p1
	env.p2=p2
end 
local function func(input,seg,env)
	for cand in input:iter() do 
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

