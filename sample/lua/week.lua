--[[
week_translator: 将 `week` 翻译为当前星期
--]]

local function translator(input, seg)
   if (input == "week") then
      if (os.date("%w") == "0") then
         weekstr = "日"
      end
      if (os.date("%w") == "1") then
         weekstr = "一"
      end
      if (os.date("%w") == "2") then
         weekstr = "二"
      end
      if (os.date("%w") == "3") then
         weekstr = "三"
      end
      if (os.date("%w") == "4") then
         weekstr = "四"
      end
      if (os.date("%w") == "5") then
         weekstr = "五"
      end
      if (os.date("%w") == "6") then
         weekstr = "六"
      end
      yield(Candidate("week", seg.start, seg._end, "".."星期"..weekstr.."","星期"))
      yield(Candidate("week", seg.start, seg._end, os.date("%Y年%m月%d日").."".."星期"..weekstr.."","星期"))
      yield(Candidate("week", seg.start, seg._end, os.date("%Y年%m月%d日").."".."星期"..weekstr..""..os.date("%H:%M:%S"),"星期"))
   end
end

return translator
