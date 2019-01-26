-- Usage:
--  engine:
--    ...
--    translators:
--      ...
--      - lua_translator@lua_function3
--      - lua_translator@lua_function4
--      ...
--    filters:
--      ...
--      - lua_filter@lua_function1
--      - lua_filter@lua_function2
--      ...

--- date/time translator
function date_translator(input, seg)
   if (input == "/date") then
      -- Candidate(type, start, end, text, comment)
      yield(Candidate("date", seg.start, seg._end, os.date("%Y年%m月%d日"), " 日期"))
   end
end

function time_translator(input, seg)
   if (input == "/time") then
      yield(Candidate("time", seg.start, seg._end, os.date("%H:%M:%S"), " 时间"))
   end
end


--- charset filter
charset = {
   ["CJK"] = { first = 0x4E00, last = 0x9FFF },
   ["ExtA"] = { first = 0x3400, last = 0x4DBF },
   ["ExtB"] = { first = 0x20000, last = 0x2A6DF },
   ["ExtC"] = { first = 0x2A700, last = 0x2B73F },
   ["ExtD"] = { first = 0x2B740, last = 0x2B81F },
   ["ExtE"] = { first = 0x2B820, last = 0x2CEAF },
   ["ExtF"] = { first = 0x2CEB0, last = 0x2EBEF },
   ["Compat"] = { first = 0x2F800, last = 0x2FA1F } }

function exists(single_filter, text)
  for i in utf8.codes(text) do
     local c = utf8.codepoint(text, i)
     if (not single_filter(c)) then
	return false
     end
  end
  return true
end

function is_charset(s)
   return function (c)
      return c >= charset[s].first and c <= charset[s].last
   end
end

function is_cjk_ext(c)
   return is_charset("ExtA")(c) or is_charset("ExtB")(c) or
      is_charset("ExtC")(c) or is_charset("ExtD")(c) or
      is_charset("ExtE")(c) or is_charset("ExtF")(c) or
      is_charset("Compat")(c)
end

function charset_filter(input)
   for cand in input:iter() do
      if (not exists(is_cjk_ext, cand.text))
      then
	 yield(cand)
      end
   end
end


--- charset comment filter
function charset_comment_filter(input)
   for cand in input:iter() do
      for s, r in pairs(charset) do
	 if (exists(is_charset(s), cand.text)) then
	    cand:get_genuine().comment = cand.comment .. " " .. s
	    break
	 end
      end
      yield(cand)
   end
end


--- single_char_filter
function single_char_filter(input)
   local l = {}
   for cand in input:iter() do
      if (utf8.len(cand.text) == 1) then
	 yield(cand)
      else
	 table.insert(l, cand)
      end
   end
   for i, cand in ipairs(l) do
      yield(cand)
   end
end


--- reverse_lookup_filter
pydb = ReverseDb("build/terra_pinyin.reverse.bin")

function xform_py(inp)
   if inp == "" then return "" end
   inp = string.gsub(inp, "([aeiou])(ng?)([1234])", "%1%3%2")
   inp = string.gsub(inp, "([aeiou])(r)([1234])", "%1%3%2")
   inp = string.gsub(inp, "([aeo])([iuo])([1234])", "%1%3%2")
   inp = string.gsub(inp, "a1", "ā")
   inp = string.gsub(inp, "a2", "á")
   inp = string.gsub(inp, "a3", "ǎ")
   inp = string.gsub(inp, "a4", "à")
   inp = string.gsub(inp, "e1", "ē")
   inp = string.gsub(inp, "e2", "é")
   inp = string.gsub(inp, "e3", "ě")
   inp = string.gsub(inp, "e4", "è")
   inp = string.gsub(inp, "o1", "ō")
   inp = string.gsub(inp, "o2", "ó")
   inp = string.gsub(inp, "o3", "ǒ")
   inp = string.gsub(inp, "o4", "ò")
   inp = string.gsub(inp, "i1", "ī")
   inp = string.gsub(inp, "i2", "í")
   inp = string.gsub(inp, "i3", "ǐ")
   inp = string.gsub(inp, "i4", "ì")
   inp = string.gsub(inp, "u1", "ū")
   inp = string.gsub(inp, "u2", "ú")
   inp = string.gsub(inp, "u3", "ǔ")
   inp = string.gsub(inp, "u4", "ù")
   inp = string.gsub(inp, "v1", "ǖ")
   inp = string.gsub(inp, "v2", "ǘ")
   inp = string.gsub(inp, "v3", "ǚ")
   inp = string.gsub(inp, "v4", "ǜ")
   inp = string.gsub(inp, "([nljqxy])v", "%1ü")
   inp = string.gsub(inp, "eh[0-5]?", "ê")
   return "(" .. inp .. ")"
end

function reverse_lookup_filter(input)
   for cand in input:iter() do
      cand:get_genuine().comment = cand.comment .. " " .. xform_py(pydb:lookup(cand.text))
      yield(cand)
   end
end


--- composition
function myfilter(input)
   local input2 = Translation(charset_comment_filter, input)
   reverse_lookup_filter(input2)
end

function mytranslator(input, seg)
   date_translator(input, seg)
   time_translator(input, seg)
end
