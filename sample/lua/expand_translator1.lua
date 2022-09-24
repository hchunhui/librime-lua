

--[[
in cangjie5.schema.yaml

schema:
  schema_id: cangjie5
engine:
  translators:
    - lua_translator@expand_translator
]]

local function memoryCallback(memory, commit)
	for i,dictentry in ipairs(commit:get())
	do
		log.info(dictentry.text .. " " .. dictentry.weight .. " " .. dictentry.comment .. "")
		memory:update_userdict(dictentry,0,"") -- do nothing to userdict
		-- memory:update_userdict(dictentry,1,"") -- update entry to userdict
		-- memory:update_userdict(dictentry,1,"") -- delete entry to userdict
	end
	return true
end

local function init(env)
   env.mem = Memory(env.engine,Schema("luna_pinyin"),'translator', memoryCallback) --
   config = env.engine.schema.config
   env.tag = config:get_string(env.name_space .. '/tag') or 'abc'
   namespace = 'expand_translator'
   log.info("expand_translator Initilized!")
end

-- translation function
-- 使用 Translation( func, argvs....)
local function transltion_func(inp,seg,mem)
   if mem:dict_lookup(inp, true, 100) then
     for entry in mem:iter_dict() do
       local code = mem:decode(entry.code)
       local codeComment = table.concat(code, ',')
       local ph = Phrase(mem, 'expand_translator', seg.start, seg._end, entry)
       ph.comment = codeComment
       yield(ph:toCandidate())
     end
   end
 end
local function translation_func1(translation,str)
  for cand in translation:iter() do
    local gcand=cand:get_genuine()
    gcand.comment = gcand.comment .. ":" .. str
    yield(cand)
  end
end

local function translate(inp,seg,env)
  if not seg:has_tag(env.tag) then return end
  -- t1 memory 請調出 cand
  local active_inp= inp:sub(seg.start+1, seg._end)
  local t1 = Translator( translation_func, active_inp, seg, mem)
  -- t2 修改 candidate comment
  local t2 = Translator( translation_func1, t1,'--test--')
  for cand in t2:iter() do
    yield(cand)
  end
end

return {init = init, func = translate}
