#! /usr/bin/env lua
--
-- main_init.lua
-- Copyright (C) 2020 Shewer Lu <shewer@gmail.com>
--
-- Distributed under terms of the MIT license.
--
--------------------------------------------

--  lua_init(argv) argv 自定義
local Notifier=require ('tools/notifier') -- 建立 notifier obj   減化 notifier connect and disconnect() 
-- 取得 librime 狀態 tab { always=true ....}
local function status(ctx)
    local stat=metatable()
    local comp= ctx.composition
    stat.always=true
    stat.composing= ctx:is_composing()
    stat.empty= not stat.composing
    stat.has_menu= ctx:has_menu()
    stat.paging= not comp:empty() and comp:back():has_tag("paging")
    return stat
end

local function pressor_key(bindkey, keys, env)
		env.key[bindkey]=true
		env.engine:process_keys(keys)
		env.key[bindkey]=nil
end 

local function lua_init(...)
	local args={...} 

	local function processor_func(key,env) -- key:KeyEvent,env_
		local Rejected, Accepted, Noop = 0,1,2 
		local engine=env.engine
		local context=engine.context
		local s= status(context) 

		if s.empty then end 
		if s.always then 
			-- 切換 狀態  井在 translation  加入 option_update_notifier 修改 tran1 tran2 狀態
			if key:repr() == "F7" then 
				context:set_option("switch_dict", not context:get_option("switch_dict")) 
		    end 
			if key:repr() == "F8" then 
				context:set_option("all_on",not context:get_option("all_on")) 
		    end 
			if key:repr() == "F9" then 
				context:set_option("completeion",not context:get_option("completeion")) 
		    end 

		end 
		if s.has_menu then end 
		if s.composing then end 
		if s.paging then end 



		if not context.composition:empty() then 
			
		end 
		return Noop  
	end 

	local function processor_init_func(env)
		env.key={}

	end 

	local function processor_fini_func(env) 
		env.connect:disconnect() -- 將所有 connection  disconnect() 
	end 

	-- segmentor 
	local function segmentor_func(segs ,env) -- segmetation:Segmentation,env_


	-- 終止 後面 segmentor   打tag  
	-- return  true next segmentor check
		return true 
	end 
	local function segmentor_init_func(env)
	end 
	local function segmentor_fini_func(env)
	end 

	-- translator 
	local function translator_func(input,seg,env)  -- input:string, seg:Segment, env_
		--tran:append( env.tr:query("ab",seg) ) 
		
		assert(env.tabtr1,env.tabtr1)
		assert(env.tabtr1,env.tabtr2)
		
        -- 準備 建立 transltion 
		--  :iter()  只能處理一次，所以需要將 translation 合併
		local t1= env.tabtr1.translator:query(""..input,seg)
		local t2= env.tabtr2:query(""..input,seg)
		--print_type(t1,"--t1:")
		--print_type(tt1," Tran :")

		local tran= UnionTranslation()
		local tran1=FifoTranslation()  
		tran1:append(  Candidate("test", seg.start,seg._end, "test", "test") )


		--local tran= MergedTranslation()

		assert(tran,tran)
		print_type(tran," Merged :")
		local tt= 0< tran1.size  and tran:append( tran1.translation ) -- tran1.translation ( converted to Translation )  
		local tt= t1 and tran:append( t1) 
		tt= t2 and tran:append( t2) 
		print("--------before----in iter---")
		for c1 in  tran:iter() do 
			-- generate LuaTranslation 
			yield(c1)
		end 
	end 
	
	local function translator_init_func(env)
 		--載入 TableTranslator 
		-- register 2 translatior   (engine, name_space) 
		local ticket1= Ticket(env.engine,"cangjie5liu")
		local ticket2= Ticket(env.engine,"translator")
		assert(ticket1, ticket1) 
		assert(ticket2, ticket2) 
		env.tabtr1= TableTranslator(ticket1)
		env.tabtr2= TableTranslator(ticket2)
		assert(env.tabtr1,env.tabtr1)
		assert(env.tabtr1,env.tabtr2)
		-------設定 tag "abc"  -------------------------------
		env.tabtr1.option.tag="abc"
		env.tabtr2.option.tag="abc"
		-- using notifier to modify  translator  status 
		env.connection=env.engine.context.option_update_notifier:connect( 
		  function (ctx,name)
			if name== "switch_dict" then 
				ctx:refresh_non_confirmed_composition()
				if ctx:get_option(name) then 
					env.tabtr1.option.tag="ab1"
					env.tabtr2.option.tag="abc"
				else
					env.tabtr1.option.tag="abc"
					env.tabtr2.option.tag="ab1"
				
				end 
				print( name, ctx:get_option(name) )
			end 
			if name == "all_on" then 
				ctx:refresh_non_confirmed_composition()
				print( name, ctx:get_option(name) )
				if ctx:get_option(name) then 
					env.tabtr1.option.tag="abc"
					env.tabtr2.option.tag="abc"
				else 
					env.tabtr1.option.tag="ab1"
					env.tabtr2.option.tag="ab1"
				end 
			end 
			if name == "completeion" then 
				ctx:refresh_non_confirmed_composition()
				print( name, ctx:get_option(name) )
				if ctx:get_option(name) then 
					env.tabtr1.option.enable_completion=true
					env.tabtr2.option.enable_completion=true
				else 
					env.tabtr1.option.enable_completion=false
					env.tabtr2.option.enable_completion=false
				end 
			end 
			print( 
			  ("-------switch_dict: %s,\t all_on : %s \t completeion %s"):format( 
			ctx:get_option("switch_dict"), ctx:get_option("all_on") , ctx:get_option("completeion") )
			)


		end )



	end 
	local function translator_fini_func(env)
		env.connection:disconnect() 
	

	end 

	--- filter  
	local function filter_func(input,seg,env)   -- pass filter 
		for cand in input:iter() do 
			yield(cand)
		end 
	end 
	local function filter_init_func(env)
	end 
	local function filter_fini_func(env)
	end 

	local _tab= { 
		processor= { func=processor_func, init=processor_init_func, fini=processor_fini_func} , 
		--segmentor= { func= segmentor_func, init=segmentor_init_func , fini=segmentor_fini_func} , 
		translator={ func=translator_func, init=translator_init_func,fini=translator_fini_func} , 
		-- filter=    { func=filter_func, init=filter_init_func,    fini=filter_fini_func } ,   
		--filter1=    { func=filter_func1, init=filter_init_func1,    fini=filter_fini_func1 } ,   
	}
	return _tab
end 





return lua_init    
