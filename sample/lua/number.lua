--[[
number_translator: 将 `/` + 阿拉伯数字 翻译为大小写汉字
--]]

local confs = {
  {
    comment = " 大写",
    numeral = { [0] = "零", "壹", "贰", "叁", "肆", "伍", "陆", "柒", "捌", "玖" },
    place = { [0] = "", "拾", "佰", "仟" },
    group = { [0] = "", "万", "亿", "万亿", "亿亿" }
  },
  {
    comment = " 小写",
    numeral = { [0] = "零", "一", "二", "三", "四", "五", "六", "七", "八", "九" },
    place = { [0] = "", "十", "百", "千" },
    group = { [0] = "", "万", "亿", "万亿", "亿亿" }
  },
  {
    comment = " 序数",
    numeral = { [0] = "〇", "一", "二", "三", "四", "五", "六", "七", "八", "九" }
  },
  {
    comment = " 大寫",
    numeral = { [0] = "零", "壹", "貳", "參", "肆", "伍", "陸", "柒", "捌", "玖" },
    place = { [0] = "", "拾", "佰", "仟" },
    group = { [0] = "", "萬", "億", "兆", "京" }
  },
  {
    comment = " 小寫",
    numeral = { [0] = "零", "一", "二", "三", "四", "五", "六", "七", "八", "九" },
    place = { [0] = "", "十", "百", "千" },
    group = { [0] = "", "萬", "億", "兆", "京" }
  },
  {
    comment = " 序數",
    numeral = { [0] = "〇", "一", "二", "三", "四", "五", "六", "七", "八", "九" }
  }
}

local function read_seg(conf, n)
  local s = ""
  local i = 0
  local zf = true

  while string.len(n) > 0 do
    local d = tonumber(string.sub(n, -1, -1))
    if conf.place == nil then
      s = conf.numeral[d] .. s
    elseif d == 1 and i == 1 and string.len(n) == 1 then
      s = conf.place[i] .. s
    elseif d ~= 0 then
      s = conf.numeral[d] .. conf.place[i] .. s
      zf = false
    else
      if not zf then
        s = conf.numeral[0] .. s
      end
      zf = true
    end
    i = i + 1
    n = string.sub(n, 1, -2)
  end

  return i < 4, s
end

local function read_number(conf, n)
  local s = ""
  local i = 0
  local zf = false

  n = string.gsub(n, "^0+", "")

  if n == "" then
    return conf.numeral[0]
  end

  while string.len(n) > 0 do
    local zf2, r = read_seg(conf, string.sub(n, -4, -1))
    if r ~= "" then
      if conf.group == nil then
        s = r .. s
      elseif zf and s ~= "" then
        s = r .. conf.group[i] .. conf.numeral[0] .. s
      else
        s = r .. conf.group[i] .. s
      end
    end
    zf = zf2
    i = i + 1
    n = string.sub(n, 1, -5)
  end
  return s
end

local function translator(input, seg)
  if string.sub(input, 1, 1) == "/" then
    local n = string.sub(input, 2)
    if tonumber(n) ~= nil then
      for _, conf in ipairs(confs) do
        local r = read_number(conf, n)
        yield(Candidate("number", seg.start, seg._end, r, conf.comment))
      end
    end
  end
end

return translator
