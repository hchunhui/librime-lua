#! /usr/bin/env lua
--
-- commit_history.lua
-- Copyright (C) 2022 Shewer Lu <shewer@gmail.com>
--
-- Distributed under terms of the MIT license.
--
-- lua_translator@commit_history@history
-- 
-- <schema_id>.custom.yaml
-- patch:
--    engine/translators/+:
--      - lua_translator@commit_history@hintory
--    history:  #  h
--      tag: abc
--      size: 10
--      input: z
--      initial_quality: 1000
--

local M = {}
function M.init(env)
  env.tag = 'abc'
  env.size = 9;
  env.input = 'z';
  env.quality = 1000;
  env.history_translatior = Component.Translator(env.engine,"translator", "history_translator")
end

function M.fini(env)
end

function M.func(inp,seg,env)
  if not seg:has_tag(env.tag) or not  inp:match("^".. env.input .. "$") then
    return
  end
  local context=env.engine.context
  local count = env.size
  --  期望
  for it, h_record in context.commit_history:iter() do 
    yield( Candidate(h_record.type, seg.start, seg._end, hrecord.text, "history") )
    count = count -1
    if count <=0 then break end
  end

end

return M
