-- config in schema
--[[
engine:
  processors:
    - lua_processor@associate_processor
  segmentors:
    - ascii_segmentor
    - matcher
    - abc_segmentor
    - punct_segmentor
    - lua_segmentor@associate_segmentor
    - fallback_segmentor
  translators:
    - lua_translator@associate_translator

associate:
  dictionary: purevocab
  result_limit: 3000
  hotkey: Control+b # defined in rime key map


]]
-- global varible
local startAssociate = false
local env = nil
local config = nil
local dictionary = nil 
local resultLimit = nil
local namespace = "associate"

local function createMenu()
			comp = env.engine.context.composition -- get current composition
			menu = Menu()
			cuSeg = comp.back()
			cuSeg.menu = menu
			-- cuSeg.menu:AddTranslation(Translation)
end	

local function processor()
	hotkey = nil

	function initialize(rimeEnv)
		env = rimeEnv
		config = env.engine.schema.config
		dictionary = config:get_string(namespace .. "/dictionary")
		resultLimit = config:get_int(namespace .. "/result_limit")
		hotkey_str = config:get_string(namespace .. "/hotkey")
		kv = KeySequence()
		kv:parse("{" .. hotkey_str .."}")
		for k,v in pairs(kv:toKeyEvent())
		do
			hotkey = v
			break
		end
		-- print("processor initialized!")
	end

	function processKey(keyEvent,rimeEnv)
		env = rimeEnv
		if hotkey ~= nil then
			if hotkey:eq(keyEvent)
			then
				startAssociate = true
				env.engine.context.input = " "
				return 1
			else
				return 2
			end
		end
		return 2
	end

	return {init=initialize, func=processKey, fini=nil}
end


local function translator()
	local previousCommit = ""
	local dict = nil
	local currentMem = nil
	local recorded = ""

	function initialize(rimeEnv)
		env = rimeEnv
		config = env.engine.schema.config
		if dictionary == nil then
			dictionary = config:get_string(namespace .. "/dictionary")
			resultLimit = config:get_string(namespace .. "/result_limit")
		end
		dict = CustomMemory(env.engine, dictionary,"")
		
		-- pass context to callback
		env.engine.context.commit_notifier:connect(record)
		-- print("translator initialized!")
	end

	function record( context )
		prefix = context:get_commit_text()
		if string.len(prefix) > 1
			then
				recorded = recorded .. prefix
				-- print(recorded)
			end
	end

	function query(input,segment,rimeEnv)
		env = rimeEnv 
		if startAssociate and recorded ~= "" then
			cnt = 0
			dict:dict_lookup(recorded,true, resultLimit)
			for entry in dict:iter_dict()
			do
				cand = string.sub(entry.text,string.len(recorded))
				if string.len(cand) > 1 then
					yield(Candidate("associate",0,1,cand,""))
					cnt = cnt + 1
					if cnt >= resultLimit then
						break
					end
				end
			end

			startAssociate = false
			recorded = ""
		end
	end
	
	return {init=initialize, func=query, fini=nil}
end

local function segmentor()
	function initialize(rimeEnv)
		env = rimeEnv
		-- print("segmentor init!")
	end

	function process( segments, rimeEnv )
		env = rimeEnv
		if startAssociate then
			anSeg = Segment(0,1)	-- reconize the Segment
			anSeg.tags = Set({"associate"})	-- to enable selector
			segments:forward()
			segments:add_segment(anSeg)
			return false
		end
		return true
	end

	return {init=initialize, func=process, fini=nil}
end


return { processor=processor(), translator=translator(), segmentor=segmentor() }