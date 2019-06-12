--[[
time_translator: 将 `time` 翻译为当前时间
--]]

local function translator(input, seg)
   if (input == "time") then
      yield(Candidate("time", seg.start, seg._end, os.date("%H:%M:%S"), " 时间"))
   end
end

return translator
