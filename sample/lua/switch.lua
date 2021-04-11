--[[
switch_processor: 通过选择自定义的候选项来切换开关（以简繁切换和下一方案为例）

`Switcher` 适用于：
1. 不方便或不倾向用 key_binder 处理的情况
2. 自定义开关的读取（本例未体现）

须将 lua_processor@switch_processor 放在 engine/processors 里，并位于默认 selector 之前

为更好的使用本例，可以添加置顶的自定义词组，如
〔简〕	simp
〔繁〕	simp
〔下一方案〕	next
--]]

-- 帮助函数，返回被选中的候选的索引
local function select_index(key, env)
  local ch = key.keycode
  local index = -1
  local select_keys = env.engine.schema.select_keys
  if select_keys ~= nil and select_keys ~= "" and not key.ctrl() and ch >= 0x20 and ch < 0x7f then
    local pos = string.find(select_keys, string.char(ch))
    if pos ~= nil then index = pos end
  elseif ch >= 0x30 and ch <= 0x39 then
    index = (ch - 0x30 + 9) % 10
  elseif ch >= 0xffb0 and ch < 0xffb9 then
    index = (ch - 0xffb0 + 9) % 10
  elseif ch == 0x20 then
    index = 0
  end
  return index
end

-- 切换开关函数
local function apply_switch(env, keyword, target_state)
  local ctx = env.engine.context
  local swt = env.switcher
  local conf = swt.user_config
  ctx:set_option(keyword, target_state)
  -- 如果设置了自动保存，则需相应的配置
  if swt:is_auto_save(keyword) and conf ~= nil then
    conf:set_bool("var/option/" .. keyword, target_state)
  end
end

local kRejected = 0
local kAccepted = 1
local kNoop = 2

local function selector(key, env)
  if env.switcher == nil then return kNoop end
  if key:release() or key:alt() then return kNoop end
  local idx = select_index(key,env)
  if idx < 0 then return kNoop end
  local ctx = env.engine.context
  if ctx.input == "simp" then -- 当输入为 "simp" 时响应选择
    local state = nil
    if idx == 0 then
      state = true
    elseif idx == 1 then
      state = false
    end
    if state ~= nil then
      apply_switch(env, "simplification", state)
      ctx:clear() -- 切换完成后清空，避免上屏
      return kAccepted
    end
  elseif ctx.input == "next" and idx == 0 then
    env.switcher:select_next_schema()
    ctx:clear()
    return kAccepted
  end

  return kNoop
end

-- 初始化 switcher
local function init(env)
  -- 若当前 librime-lua 版本未集成 Switcher 则无事发生
  if Switcher == nil then return end
  env.switcher = Switcher(env.engine)
end

return { init = init, func = selector }
