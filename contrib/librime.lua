-- Last Change: 2025-06-18
---@meta rime

--- 全局对象

---@class RimeAPI
---@field get_rime_version fun(): string
---@field get_shared_data_dir fun(): string
---@field get_user_data_dir fun(): string
---@field get_sync_dir fun(): string
---@field get_distribution_name fun(): string
---@field get_distribution_code_name fun(): string
---@field get_distribution_version fun(): string
---@field get_user_id fun(): string
---@field get_time_ms fun(): number
---@field regex_match fun(input: string, pattern: string): boolean
---@field regex_search fun(input: string, pattern: string): string[] | nil
---@field regex_replace fun(input: string, pattern: string, fmt: string): string
---@field keysym KeysymTable -- X11 键盘符号常量表
rime_api = {}

---@class Log
---@field info fun(string)
---@field warning fun(string)
---@field error fun(string)
log = {}

---@param cand Candidate
function yield(cand) end

--- 常量

---@enum ConfigType
local config_types = {
  kNull = "kNull",
  kScalar = "kScalar",
  kList = "kList",
  kMap = "kMap",
}

---@enum SegmentType
local segment_types = {
  kVoid = "kVoid",
  kGuess = "kGuess",
  kSelected = "kSelected",
  kConfirmed = "kConfirmed",
}

---@enum CandidateDynamicType
local candidate_dynamic_types = {
  kSentence = "Sentence",
  kPhrase = "Phrase",
  kSimple = "Simple",
  kShadow = "Shadow",
  kUniquified = "Uniquified",
  kOther = "Other",
}

---@enum ProcessResult
local process_results = {
  kRejected = 0,
  kAccepted = 1,
  kNoop = 2,
}

---@enum ModifierMask
local modifier_masks = {
  kShift = 0x1,
  kLock = 0x2,
  kControl = 0x4,
  kAlt = 0x8,
}

--- X11 键盘符号常量
--- 通过 rime_api.keysym 访问，包含所有 X11 键盘符号定义
--- 常用示例：
--- rime_api.keysym.XK_Return     -- 回车键
--- rime_api.keysym.XK_BackSpace  -- 退格键
--- rime_api.keysym.XK_Tab        -- Tab键
--- rime_api.keysym.XK_Escape     -- Esc键
--- rime_api.keysym.XK_space      -- 空格键
--- rime_api.keysym.XK_a          -- 小写字母a
--- rime_api.keysym.XK_A          -- 大写字母A
--- rime_api.keysym.XK_F1         -- F1功能键
--- rime_api.keysym.XK_Left       -- 左箭头键
--- rime_api.keysym.XK_Right      -- 右箭头键
--- rime_api.keysym.XK_Up         -- 上箭头键
--- rime_api.keysym.XK_Down       -- 下箭头键
--- rime_api.keysym.XK_Home       -- Home键
--- rime_api.keysym.XK_End        -- End键
--- rime_api.keysym.XK_Page_Up    -- Page Up键
--- rime_api.keysym.XK_Page_Down  -- Page Down键
--- 以及更多特殊符号、数字键盘、功能键等...

---@class KeysymTable
---@field XK_BackSpace 0xff08|65288 -- 退格键
---@field XK_Tab 0xff09|65289 -- Tab键
---@field XK_Linefeed 0xff0a|65290 -- 换行键
---@field XK_Clear 0xff0b|65291 -- 清除键
---@field XK_Return 0xff0d|65293 -- 回车键
---@field XK_Pause 0xff13|65299 -- 暂停键
---@field XK_Scroll_Lock 0xff14|65300 -- 滚动锁定键
---@field XK_Sys_Req 0xff15|65301 -- 系统请求键
---@field XK_Escape 0xff1b|65307 -- Esc键
---@field XK_Delete 0xffff|65535 -- 删除键
---@field XK_Multi_key 0xff20|65312 -- 多键组合键
---@field XK_Codeinput 0xff37|65335 -- 代码输入键
---@field XK_SingleCandidate 0xff3c|65340 -- 单候选键
---@field XK_MultipleCandidate 0xff3d|65341 -- 多候选键
---@field XK_PreviousCandidate 0xff3e|65342 -- 前一候选键
---@field XK_Kanji 0xff21|65313 -- 日文汉字键
---@field XK_Muhenkan 0xff22|65314 -- 日文无变换键
---@field XK_Henkan_Mode 0xff23|65315 -- 日文变换模式键
---@field XK_Henkan 0xff23|65315 -- 日文变换键
---@field XK_Romaji 0xff24|65316 -- 日文罗马字键
---@field XK_Hiragana 0xff25|65317 -- 日文平假名键
---@field XK_Katakana 0xff26|65318 -- 日文片假名键
---@field XK_Hiragana_Katakana 0xff27|65319 -- 日文平假名片假名切换键
---@field XK_Zenkaku 0xff28|65320 -- 日文全角键
---@field XK_Hankaku 0xff29|65321 -- 日文半角键
---@field XK_Zenkaku_Hankaku 0xff2a|65322 -- 日文全角半角切换键
---@field XK_Touroku 0xff2b|65323 -- 日文登录键
---@field XK_Massyo 0xff2c|65324 -- 日文抹消键
---@field XK_Kana_Lock 0xff2d|65325 -- 日文假名锁定键
---@field XK_Kana_Shift 0xff2e|65326 -- 日文假名切换键
---@field XK_Eisu_Shift 0xff2f|65327 -- 日文英数切换键
---@field XK_Eisu_toggle 0xff30|65328 -- 日文英数切换键
---@field XK_Home 0xff50|65360 -- Home键
---@field XK_Left 0xff51|65361 -- 左箭头键
---@field XK_Up 0xff52|65362 -- 上箭头键
---@field XK_Right 0xff53|65363 -- 右箭头键
---@field XK_Down 0xff54|65364 -- 下箭头键
---@field XK_Prior 0xff55|65365 -- Page Up键（旧名）
---@field XK_Page_Up 0xff55|65365 -- Page Up键
---@field XK_Next 0xff56|65366 -- Page Down键（旧名）
---@field XK_Page_Down 0xff56|65366 -- Page Down键
---@field XK_End 0xff57|65367 -- End键
---@field XK_Begin 0xff58|65368 -- Begin键
---@field XK_Select 0xff60|65376 -- 选择键
---@field XK_Print 0xff61|65377 -- 打印键
---@field XK_Execute 0xff62|65378 -- 执行键
---@field XK_Insert 0xff63|65379 -- 插入键
---@field XK_Undo 0xff65|65381 -- 撤销键
---@field XK_Redo 0xff66|65382 -- 重做键
---@field XK_Menu 0xff67|65383 -- 菜单键
---@field XK_Find 0xff68|65384 -- 查找键
---@field XK_Cancel 0xff69|65385 -- 取消键
---@field XK_Help 0xff6a|65386 -- 帮助键
---@field XK_Break 0xff6b|65387 -- 中断键
---@field XK_Mode_switch 0xff7e|65390 -- 模式切换键
---@field XK_script_switch 0xff7e|65390 -- 脚本切换键
---@field XK_Num_Lock 0xff7f|65407 -- 数字锁定键
---@field XK_KP_Space 0xff80|65408 -- 小键盘空格键
---@field XK_KP_Tab 0xff89|65417 -- 小键盘Tab键
---@field XK_KP_Enter 0xff8d|65421 -- 小键盘回车键
---@field XK_KP_F1 0xff91|65425 -- 小键盘F1键
---@field XK_KP_F2 0xff92|65426 -- 小键盘F2键
---@field XK_KP_F3 0xff93|65427 -- 小键盘F3键
---@field XK_KP_F4 0xff94|65428 -- 小键盘F4键
---@field XK_KP_Home 0xff95|65429 -- 小键盘Home键
---@field XK_KP_Left 0xff96|65430 -- 小键盘左箭头键
---@field XK_KP_Up 0xff97|65431 -- 小键盘上箭头键
---@field XK_KP_Right 0xff98|65432 -- 小键盘右箭头键
---@field XK_KP_Down 0xff99|65433 -- 小键盘下箭头键
---@field XK_KP_Prior 0xff9a|65434 -- 小键盘Page Up键
---@field XK_KP_Page_Up 0xff9a|65434 -- 小键盘Page Up键
---@field XK_KP_Next 0xff9b|65435 -- 小键盘Page Down键
---@field XK_KP_Page_Down 0xff9b|65435 -- 小键盘Page Down键
---@field XK_KP_End 0xff9c|65436 -- 小键盘End键
---@field XK_KP_Begin 0xff9d|65437 -- 小键盘Begin键
---@field XK_KP_Insert 0xff9e|65438 -- 小键盘插入键
---@field XK_KP_Delete 0xff9f|65439 -- 小键盘删除键
---@field XK_KP_Equal 0xffbd|65469 -- 小键盘等号键
---@field XK_KP_Multiply 0xffaa|65450 -- 小键盘乘号键
---@field XK_KP_Add 0xffab|65451 -- 小键盘加号键
---@field XK_KP_Separator 0xffac|65452 -- 小键盘分隔符键
---@field XK_KP_Subtract 0xffad|65453 -- 小键盘减号键
---@field XK_KP_Decimal 0xffae|65454 -- 小键盘小数点键
---@field XK_KP_Divide 0xffaf|65455 -- 小键盘除号键
---@field XK_KP_0 0xffb0|65456 -- 小键盘数字0
---@field XK_KP_1 0xffb1|65457 -- 小键盘数字1
---@field XK_KP_2 0xffb2|65458 -- 小键盘数字2
---@field XK_KP_3 0xffb3|65459 -- 小键盘数字3
---@field XK_KP_4 0xffb4|65460 -- 小键盘数字4
---@field XK_KP_5 0xffb5|65461 -- 小键盘数字5
---@field XK_KP_6 0xffb6|65462 -- 小键盘数字6
---@field XK_KP_7 0xffb7|65463 -- 小键盘数字7
---@field XK_KP_8 0xffb8|65464 -- 小键盘数字8
---@field XK_KP_9 0xffb9|65465 -- 小键盘数字9
---@field XK_F1 0xffbe|65470 -- F1功能键
---@field XK_F2 0xffbf|65471 -- F2功能键
---@field XK_F3 0xffc0|65472 -- F3功能键
---@field XK_F4 0xffc1|65473 -- F4功能键
---@field XK_F5 0xffc2|65474 -- F5功能键
---@field XK_F6 0xffc3|65475 -- F6功能键
---@field XK_F7 0xffc4|65476 -- F7功能键
---@field XK_F8 0xffc5|65477 -- F8功能键
---@field XK_F9 0xffc6|65478 -- F9功能键
---@field XK_F10 0xffc7|65479 -- F10功能键
---@field XK_F11 0xffc8|65480 -- F11功能键
---@field XK_F12 0xffc9|65481 -- F12功能键
---@field XK_F13 0xffca|65482 -- F13功能键
---@field XK_F14 0xffcb|65483 -- F14功能键
---@field XK_F15 0xffcc|65484 -- F15功能键
---@field XK_F16 0xffcd|65485 -- F16功能键
---@field XK_F17 0xffce|65486 -- F17功能键
---@field XK_F18 0xffcf|65487 -- F18功能键
---@field XK_F19 0xffd0|65488 -- F19功能键
---@field XK_F20 0xffd1|65489 -- F20功能键
---@field XK_F21 0xffd2|65490 -- F21功能键
---@field XK_F22 0xffd3|65491 -- F22功能键
---@field XK_F23 0xffd4|65492 -- F23功能键
---@field XK_F24 0xffd5|65493 -- F24功能键
---@field XK_F25 0xffd6|65494 -- F25功能键
---@field XK_F26 0xffd7|65495 -- F26功能键
---@field XK_F27 0xffd8|65496 -- F27功能键
---@field XK_F28 0xffd9|65497 -- F28功能键
---@field XK_F29 0xffda|65498 -- F29功能键
---@field XK_F30 0xffdb|65499 -- F30功能键
---@field XK_F31 0xffdc|65500 -- F31功能键
---@field XK_F32 0xffdd|65501 -- F32功能键
---@field XK_F33 0xffde|65502 -- F33功能键
---@field XK_F34 0xffdf|65503 -- F34功能键
---@field XK_F35 0xffe0|65504 -- F35功能键
---@field XK_Shift_L 0xffe1|65505 -- 左Shift键
---@field XK_Shift_R 0xffe2|65506 -- 右Shift键
---@field XK_Control_L 0xffe3|65507 -- 左Ctrl键
---@field XK_Control_R 0xffe4|65508 -- 右Ctrl键
---@field XK_Caps_Lock 0xffe5|65509 -- 大写锁定键
---@field XK_Shift_Lock 0xffe6|65510 -- Shift锁定键
---@field XK_Meta_L 0xffe7|65511 -- 左Meta键
---@field XK_Meta_R 0xffe8|65512 -- 右Meta键
---@field XK_Alt_L 0xffe9|65513 -- 左Alt键
---@field XK_Alt_R 0xffea|65514 -- 右Alt键
---@field XK_Super_L 0xffeb|65515 -- 左Super键（Windows键）
---@field XK_Super_R 0xffec|65516 -- 右Super键
---@field XK_Hyper_L 0xffed|65517 -- 左Hyper键
---@field XK_Hyper_R 0xffee|65518 -- 右Hyper键
---@field XK_space 0x0020|32 -- 空格键
---@field XK_exclam 0x0021|33 -- 感叹号 !
---@field XK_quotedbl 0x0022|34 -- 双引号 "
---@field XK_numbersign 0x0023|35 -- 井号 #
---@field XK_dollar 0x0024|36 -- 美元符号 $
---@field XK_percent 0x0025|37 -- 百分号 %
---@field XK_ampersand 0x0026|38 -- 和号 &
---@field XK_apostrophe 0x0027|39 -- 单引号 '
---@field XK_parenleft 0x0028|40 -- 左圆括号 (
---@field XK_parenright 0x0029|41 -- 右圆括号 )
---@field XK_asterisk 0x002a|42 -- 星号 *
---@field XK_plus 0x002b|43 -- 加号 +
---@field XK_comma 0x002c|44 -- 逗号 ,
---@field XK_minus 0x002d|45 -- 减号 -
---@field XK_period 0x002e|46 -- 句号 .
---@field XK_slash 0x002f|47 -- 斜杠 /
---@field XK_0 0x0030|48 -- 数字0
---@field XK_1 0x0031|49 -- 数字1
---@field XK_2 0x0032|50 -- 数字2
---@field XK_3 0x0033|51 -- 数字3
---@field XK_4 0x0034|52 -- 数字4
---@field XK_5 0x0035|53 -- 数字5
---@field XK_6 0x0036|54 -- 数字6
---@field XK_7 0x0037|55 -- 数字7
---@field XK_8 0x0038|56 -- 数字8
---@field XK_9 0x0039|57 -- 数字9
---@field XK_colon 0x003a|58 -- 冒号 :
---@field XK_semicolon 0x003b|59 -- 分号 ;
---@field XK_less 0x003c|60 -- 小于号 <
---@field XK_equal 0x003d|61 -- 等号 =
---@field XK_greater 0x003e|62 -- 大于号 >
---@field XK_question 0x003f|63 -- 问号 ?
---@field XK_at 0x0040|64 -- At符号 @
---@field XK_A 0x0041|65 -- 大写字母A
---@field XK_B 0x0042|66 -- 大写字母B
---@field XK_C 0x0043|67 -- 大写字母C
---@field XK_D 0x0044|68 -- 大写字母D
---@field XK_E 0x0045|69 -- 大写字母E
---@field XK_F 0x0046|70 -- 大写字母F
---@field XK_G 0x0047|71 -- 大写字母G
---@field XK_H 0x0048|72 -- 大写字母H
---@field XK_I 0x0049|73 -- 大写字母I
---@field XK_J 0x004a|74 -- 大写字母J
---@field XK_K 0x004b|75 -- 大写字母K
---@field XK_L 0x004c|76 -- 大写字母L
---@field XK_M 0x004d|77 -- 大写字母M
---@field XK_N 0x004e|78 -- 大写字母N
---@field XK_O 0x004f|79 -- 大写字母O
---@field XK_P 0x0050|80 -- 大写字母P
---@field XK_Q 0x0051|81 -- 大写字母Q
---@field XK_R 0x0052|82 -- 大写字母R
---@field XK_S 0x0053|83 -- 大写字母S
---@field XK_T 0x0054|84 -- 大写字母T
---@field XK_U 0x0055|85 -- 大写字母U
---@field XK_V 0x0056|86 -- 大写字母V
---@field XK_W 0x0057|87 -- 大写字母W
---@field XK_X 0x0058|88 -- 大写字母X
---@field XK_Y 0x0059|89 -- 大写字母Y
---@field XK_Z 0x005a|90 -- 大写字母Z
---@field XK_bracketleft 0x005b|91 -- 左方括号 [
---@field XK_backslash 0x005c|92 -- 反斜杠 \
---@field XK_bracketright 0x005d|93 -- 右方括号 ]
---@field XK_asciicircum 0x005e|94 -- 插入符号 ^
---@field XK_underscore 0x005f|95 -- 下划线 _
---@field XK_grave 0x0060|96 -- 反引号 `
---@field XK_a 0x0061|97 -- 小写字母a
---@field XK_b 0x0062|98 -- 小写字母b
---@field XK_c 0x0063|99 -- 小写字母c
---@field XK_d 0x0064|100 -- 小写字母d
---@field XK_e 0x0065|101 -- 小写字母e
---@field XK_f 0x0066|102 -- 小写字母f
---@field XK_g 0x0067|103 -- 小写字母g
---@field XK_h 0x0068|104 -- 小写字母h
---@field XK_i 0x0069|105 -- 小写字母i
---@field XK_j 0x006a|106 -- 小写字母j
---@field XK_k 0x006b|107 -- 小写字母k
---@field XK_l 0x006c|108 -- 小写字母l
---@field XK_m 0x006d|109 -- 小写字母m
---@field XK_n 0x006e|110 -- 小写字母n
---@field XK_o 0x006f|111 -- 小写字母o
---@field XK_p 0x0070|112 -- 小写字母p
---@field XK_q 0x0071|113 -- 小写字母q
---@field XK_r 0x0072|114 -- 小写字母r
---@field XK_s 0x0073|115 -- 小写字母s
---@field XK_t 0x0074|116 -- 小写字母t
---@field XK_u 0x0075|117 -- 小写字母u
---@field XK_v 0x0076|118 -- 小写字母v
---@field XK_w 0x0077|119 -- 小写字母w
---@field XK_x 0x0078|120 -- 小写字母x
---@field XK_y 0x0079|121 -- 小写字母y
---@field XK_z 0x007a|122 -- 小写字母z
---@field XK_braceleft 0x007b|123 -- 左花括号 {
---@field XK_bar 0x007c|124 -- 竖线 |
---@field XK_braceright 0x007d|125 -- 右花括号 }
---@field XK_asciitilde 0x007e|126 -- 波浪号 ~
---@field XK_VoidSymbol 0xffffff|16777215 -- 空符号

--- 工具

---@class Set
---@field empty fun(self: self): boolean
---@field __index function
---@field __add function
---@field __sub function
---@field __mul function
---@field __set function

---@param values any[]
---@return Set
function Set(values) end

--- 对象接口及构造函数

---@class Env
---@field engine Engine
---@field name_space string

---@class Engine
---@field schema Schema
---@field context Context
---@field active_engine Engine
---@field process_key fun(self: self, key_event: KeyEvent): boolean
---@field compose fun(self: self, ctx: Context)
---@field commit_text fun(self: self, text: string)
---@field apply_schema fun(self: self, schema: Schema)

---@class Context
---@field composition Composition
---@field input string
---@field caret_pos integer
---@field commit_notifier Notifier
---@field select_notifier Notifier
---@field update_notifier Notifier
---@field delete_notifier Notifier
---@field option_update_notifier OptionUpdateNotifier
---@field property_update_notifier PropertyUpdateNotifier
---@field unhandled_key_notifier KeyEventNotifier
---@field commit_history CommitHistory
---@field commit fun(self: self)
---@field get_commit_text fun(self: self): string
---@field get_script_text fun(self: self): string
---@field get_preedit fun(self: self): Preedit
---@field is_composing fun(self: self): boolean
---@field has_menu fun(self: self): boolean
---@field get_selected_candidate fun(self: self): Candidate
---@field push_input fun(self: self, text: string)
---@field pop_input fun(self: self, len: integer): boolean
---@field delete_input fun(self: self, len: integer): boolean
---@field clear fun(self: self)
---@field select fun(self: self, index: integer): boolean
---@field highlight fun(self: self, index: integer): boolean
---@field confirm_current_selection fun(self: self): boolean
---@field delete_current_selection fun(self: self): boolean
---@field confirm_previous_selection fun(self: self): boolean
---@field reopen_previous_selection fun(self: self): boolean
---@field clear_previous_segment fun(self: self): boolean
---@field reopen_previous_segment fun(self: self): boolean
---@field clear_non_confirmed_composition fun(self: self): boolean
---@field refresh_non_confirmed_composition fun(self: self): boolean
---@field set_option fun(self: self, name: string, value: boolean)
---@field get_option fun(self: self, name: string): boolean
---@field set_property fun(self: self, key: string, value: string)
---@field get_property fun(self: self, key: string): string
---@field clear_transient_options fun(self: self)

---@class Preedit
---@field text string
---@field caret_pos integer
---@field sel_start integer
---@field sel_end integer

---@class Composition
---@field empty fun(self: self): boolean
---@field back fun(self: self): Segment
---@field pop_back fun(self: self)
---@field push_back fun(self: self)
---@field has_finished_composition fun(self: self): boolean
---@field get_prompt fun(self: self): string
---@field toSegmentation fun(self: self): Segmentation
---@field spans fun(self: self): Spans

---@class Segmentation
---@field input string
---@field size integer
---@field empty fun(self: self): boolean
---@field back fun(self: self): Segment | nil
---@field pop_back fun(self: self)
---@field reset_length fun(self: self, length: integer)
---@field add_segment fun(self: self, seg: Segment): boolean
---@field forward fun(self: self): boolean
---@field trim fun(self: self): boolean
---@field has_finished_segmentation fun(self: self): boolean
---@field get_current_start_position fun(self: self): integer
---@field get_current_end_position fun(self: self): integer
---@field get_current_segment_length fun(self: self): integer
---@field get_confirmed_position fun(self: self): integer
---@field get_segments fun(self: self): Segment[]
---@field get_at fun(self: self, index: integer): Segment

---@class Segment
---@field status SegmentType
---@field start integer
---@field _start integer
---@field _end integer
---@field length integer
---@field tags Set
---@field menu Menu
---@field selected_index integer
---@field prompt string
---@field clear fun(self: self)
---@field close fun(self: self)
---@field reopen fun(self: self, caret_pos: integer)
---@field has_tag fun(self: self, tag: string): boolean
---@field get_candidate_at fun(self: self, index: integer): Candidate
---@field get_selected_candidate fun(self: self): Candidate
---@field active_text fun(self: self, text: string): string
---@field spans fun(self: self): Spans

---@param start_pos integer
---@param end_pos integer
---@return Segment
function Segment(start_pos, end_pos) end

---@class Spans
---@field _start integer
---@field _end integer
---@field count integer
---@field vertices integer[]
---@field add_span fun(self: self, start: integer, end: integer)
---@field add_spans fun(self: self, spans: Spans)
---@field add_vertex fun(self: self, vertex: integer)
---@field previous_stop fun(self: self, caret_pos: integer): integer
---@field next_stop fun(self: self, caret_pos: integer): integer
---@field has_vertex fun(self: self, vertex: integer): boolean
---@field count_between fun(self: self, start: integer, end: integer): integer
---@field clear fun(self: self)

---@return Spans
function Spans() end

---@class Schema
---@field schema_id string
---@field schema_name string
---@field config Config
---@field page_size integer
---@field select_keys string

---@param schema_id string
---@return Schema
function Schema(schema_id) end

---@class Config
---@field load_from_file fun(self: self, filename: string): boolean
---@field save_to_file fun(self: self, filename: string): boolean
---@field is_null fun(self: self, conf_path: string): boolean
---@field is_value fun(self: self, conf_path: string): boolean
---@field is_list fun(self: self, conf_path: string): boolean
---@field is_map fun(self: self, conf_path: string): boolean
---@field get_bool fun(self: self, conf_path: string): boolean|nil
---@field set_bool fun(self: self, conf_path: string, b: boolean): boolean
---@field get_int fun(self: self, conf_path: string): integer|nil
---@field set_int fun(self: self, conf_path: string, i: integer): boolean
---@field get_double fun(self: self, conf_path: string): number|nil
---@field set_double fun(self: self, conf_path: string, f: number): boolean
---@field get_string fun(self: self, conf_path: string): string|nil
---@field set_string fun(self: self, conf_path: string, s: string): boolean
---@field get_item fun(self: self, conf_path: string): ConfigItem|nil
---@field set_item fun(self: self, conf_path: string, item: ConfigItem): boolean
---@field get_value fun(self: self, conf_path: string): ConfigValue|nil
---@field set_value fun(self: self, conf_path: string, value: ConfigValue): boolean
---@field get_list fun(self: self, conf_path: string): ConfigList|nil
---@field set_list fun(self: self, conf_path: string, list: ConfigList): boolean
---@field get_map fun(self: self, conf_path: string): ConfigMap|nil
---@field set_map fun(self: self, conf_path: string, map: ConfigMap): boolean
---@field get_list_size fun(self: self, conf_path: string): integer|nil

---@class ConfigMap
---@field type ConfigType
---@field size integer
---@field element ConfigItem
---@field empty fun(self: self): boolean
---@field has_key fun(self: self, key: string): boolean
---@field keys fun(self: self): string[]
---@field get fun(self: self, key: string): ConfigItem|nil
---@field get_value fun(self: self, key: string): ConfigValue|nil
---@field set fun(self: self, key: string, item: ConfigItem)
---@field clear fun(self: self)

---@return ConfigMap
function ConfigMap() end

---@class ConfigList
---@field type ConfigType
---@field size integer
---@field element ConfigItem
---@field get_at fun(self: self, index: integer): ConfigItem|nil
---@field get_value_at fun(self: self, index: integer): ConfigValue|nil
---@field set_at fun(self: self, index: integer, item: ConfigItem): boolean
---@field append fun(self: self, item: ConfigItem): boolean
---@field insert fun(self: self, i: integer, item: ConfigItem): boolean
---@field clear fun(self: self): boolean
---@field empty fun(self: self): boolean
---@field resize fun(self: self, size: integer): boolean

---@return ConfigList
function ConfigList() end

---@class ConfigValue
---@field type ConfigType
---@field value string
---@field element ConfigItem
---@field get_bool fun(self: self): boolean|nil
---@field get_int fun(self: self): integer|nil
---@field get_double fun(self: self): number|nil
---@field get_string fun(self: self): string|nil
---@field set_bool fun(self: self, b: boolean)
---@field set_int fun(self: self, i: integer)
---@field set_double fun(self: self, f: number)
---@field set_string fun(self: self, s: string)

---@param value string | boolean
---@return ConfigValue
function ConfigValue(value) end

---@class ConfigItem
---@field type ConfigType
---@field empty boolean
---@field get_value fun(self: self): ConfigValue|nil
---@field get_map fun(self: self): ConfigMap|nil
---@field get_list fun(self: self): ConfigList|nil
---@field get_obj fun(self: self): ConfigMap|ConfigList|ConfigValue|nil

---@class KeyEvent
---@field keycode integer
---@field modifier integer
---@field shift fun(self: self): boolean
---@field ctrl fun(self: self): boolean
---@field alt fun(self: self): boolean
---@field caps fun(self: self): boolean
---@field super fun(self: self): boolean
---@field release fun(self: self): boolean
---@field repr fun(self: self): string
---@field eq fun(self: self, key: KeyEvent): boolean
---@field lt fun(self: self, key: KeyEvent): boolean

---@param repr string
---@return KeyEvent
function KeyEvent(repr) end

---@param keycode integer
---@param modifier integer
---@return KeyEvent
function KeyEvent(keycode, modifier) end

---@class KeySequence
---@field parse fun(self: self, repr: string): boolean
---@field repr fun(self: self): string
---@field toKeyEvent fun(self: self): KeyEvent[]

---@param repr string?
---@return KeySequence
function KeySequence(repr) end

---@class Candidate
---@field type string
---@field start integer
---@field _start integer
---@field _end integer
---@field quality number
---@field text string
---@field comment string
---@field preedit string
---@field get_dynamic_type fun(self: self): CandidateDynamicType
---@field get_genuine fun(self: self): Candidate
---@field get_genuines fun(self: self): Candidate[]
---@field to_shadow_candidate fun(self: self, type: string?, text: string?, comment: string?, inherit_comment: boolean?): ShadowCandidate
---@field to_uniquified_candidate fun(self: self, type: string?, text: string?, comment: string?): UniquifiedCandidate
---@field to_phrase fun(self: self): Phrase
---@field to_sentence fun(self: self): Sentence
---@field append fun(self: self, cand: Candidate)
---@field spans fun(self: self): Spans

---@param type string
---@param start integer
---@param _end integer
---@param text string
---@param comment string
---@return Candidate
function Candidate(type, start, _end, text, comment) end

---@class UniquifiedCandidate: Candidate

---@param candidate Candidate
---@param type string?
---@param text string?
---@param comment string?
function UniquifiedCandidate(candidate, type, text, comment) end

---@class ShadowCandidate: Candidate

---@param candidate Candidate
---@param type string?
---@param text string?
---@param comment string?
---@param inherit_comment boolean?
---@return ShadowCandidate
function ShadowCandidate(candidate, type, text, comment, inherit_comment) end

---@class Phrase
-----@field language Language 暂时不支持
---@field lang_name string
---@field type string
---@field start integer
---@field _start integer
---@field _end integer
---@field quality number
---@field text string
---@field comment string
---@field preedit string
---@field weight number
---@field code Code
---@field entry DictEntry
---@field toCandidate fun(self: self): Candidate
---@field spans fun(self: self): Spans

---@param memory Memory
---@param type string
---@param start integer
---@param _end integer
---@param entry DictEntry
---@return Phrase
function Phrase(memory, type, start, _end, entry) end

---@class Sentence
-----@field language Language 暂时不支持
---@field lang_name string
---@field type string
---@field start integer
---@field _start integer
---@field _end integer
---@field quality number
---@field text string
---@field comment string
---@field preedit string
---@field weight number
---@field code Code
---@field entry DictEntry
---@field word_lengths integer[]
---@field entrys DictEntry[]
---@field entrys_size integer
---@field entrys_empty boolean
---@field toCandidate fun(self: self): Candidate

---@class Menu
---@field add_translation fun(self: self, translation: Translation)
---@field prepare fun(self: self, candidate_count: integer): integer
---@field get_candidate_at fun(self: self, i: integer): Candidate|nil
---@field candidate_count fun(self: self): integer
---@field empty fun(self: self): boolean

---@return Menu
function Menu() end

---@class Opencc
---@field convert fun(self: self, text: string): string
---@field convert_text fun(self: self, text: string): string
---@field random_convert_text fun(self: self, text: string): string
---@field convert_word fun(self: self, text: string): string[]

---@param filename string
---@return Opencc
function Opencc(filename) end

---@class Dictionary
---@field name string
---@field loaded boolean
---@field lookup_words fun(self: self, code: string, predictive: boolean, limit: integer): boolean
---@field decode fun(self: self, code: Code): string[]

---@class DictEntryIterator
---@field exhausted boolean
---@field size integer
---@field iter fun(self: self): fun(): DictEntry|nil

---@class UserDictionary
---@field name string
---@field loaded boolean
---@field tick integer
---@field lookup_words fun(self: self, code: string, predictive: boolean, limit: integer): boolean
---@field update_entry fun(self: self, entry: DictEntry, commits: integer, prefix: string, lang_name: string): boolean

---@class UserDictEntryIterator
---@field exhausted boolean
---@field size integer
---@field iter fun(self: self): fun(): DictEntry|nil

---@class ReverseDb
---@field lookup fun(self: self, key: string): string

---@param file_name string
---@return ReverseDb
function ReverseDb(file_name) end

---@class ReverseLookup
---@field lookup fun(self: self, key: string): string
---@field lookup_stems fun(self: self, key: string): string

---@param dict_name string
---@return ReverseLookup
function ReverseLookup(dict_name) end

---@class DictEntry
---@field text string
---@field comment string
---@field preedit string
---@field weight number
---@field commit_count integer `2`
---@field custom_code string "hao", "ni hao"
---@field remaining_code_length integer "~ao"
---@field code Code

---@return DictEntry
function DictEntry() end

---@class CommitEntry: DictEntry
---@field get fun(self: self): DictEntry[]
---@field update_entry fun(self: self, entry: DictEntry, commit: integer, prefix: string): boolean
---@field update fun(self: self, commit: integer): boolean

---@class Code
---@field push fun(self: self, syllable_id: integer)
---@field print fun(self: self): string

---@return Code
function Code() end

---@class Translation
---@field exhausted boolean
---@field iter fun(self: self): fun(): Candidate|nil

function Translation() end

---@class Memory
---@field lang_name string
---@field dict Dictionary
---@field user_dict UserDictionary
---@field start_session fun(self: self): boolean
---@field finish_session fun(self: self): boolean
---@field discard_session fun(self: self): boolean
---@field dict_lookup fun(self: self, input: string, predictive: boolean, limit: integer): boolean
---@field user_lookup fun(self: self, input: string, predictive: boolean): boolean
---@field dictiter_lookup fun(self: self, input: string, predictive: boolean, limit: integer): DictEntryIterator
---@field useriter_lookup fun(self: self, input: string, predictive: boolean): UserDictEntryIterator
---@field memorize fun(self: self, callback: fun(ce: CommitEntry))
---@field decode fun(self: self, code: Code): string[]
---@field iter_dict fun(self: self): fun(): DictEntry|nil
---@field iter_user fun(self: self): fun(): DictEntry|nil
---@field update_userdict fun(self: self, entry: DictEntry, commits: integer, prefix: string): boolean
---@field update_entry fun(self: self, entry: DictEntry, commits: integer, prefix: string, lang_name?: string): boolean
---@field update_candidate fun(self: self, candidate: Candidate, commits: integer): boolean
---@field disconnect fun(self: self)

---@param engine Engine
---@param schema Schema
---@param namespace string?
---@return Memory
function Memory(engine, schema, namespace) end

---@class Projection
---@field load fun(self: self, rules: ConfigList): boolean
---@field apply fun(self: self, str: string, ret_org_str?: boolean): string

---@return Projection
function Projection() end

---@class Component
---@field Processor fun(engine: Engine, namespace: string, klass: string): Processor
---@field Translator fun(engine: Engine, namespace: string, klass: string): Translator
---@field Segmentor fun(engine: Engine, namespace: string, klass: string): Segmentor
---@field Filter fun(engine: Engine, namespace: string, klass: string): Filter
---@field ScriptTranslator fun(engine: Engine, namespace: string, klass: string): ScriptTranslator
---@field TableTranslator fun(engine: Engine, namespace: string, klass: string): TableTranslator
Component = {}

---@class Processor
---@field name_space string
---@field process_key_event fun(self: self, key_event: KeyEvent): ProcessResult

---@class Segmentor
---@field name_space string
---@field proceed fun(self: self, segmentation: Segmentation): boolean

---@class Translator
---@field name_space string
---@field query fun(self: self, input: string, segment: Segment): Translation

---@class ScriptTranslator
---@field name_space string
---@field lang_name string
---@field memorize_callback fun(ce: CommitEntry)
---@field max_homophones integer
---@field spelling_hints integer
---@field always_show_comments boolean
---@field enable_correction boolean
---@field delimiters string
---@field tag string
---@field enable_completion boolean
---@field contextual_suggestions boolean
---@field strict_spelling boolean
---@field initial_quality number
---@field preedit_formatter Projection
---@field comment_formatter Projection
---@field dict Dictionary
---@field user_dict UserDictionary
---@field translator Translator
---@field query fun(self: self, input: string, segment: Segment): Translation
---@field start_session fun(self: self): boolean
---@field finish_session fun(self: self): boolean
---@field discard_session fun(self: self): boolean
---@field memorize fun(self: self, callback: fun(ce: CommitEntry))
---@field update_entry fun(self: self, entry: DictEntry, commits: integer, prefix: string): boolean
---@field reload_user_dict_disabling_patterns fun(self: self, config_list: ConfigList): boolean
---@field set_memorize_callback fun(self: self, callback: fun(ce: CommitEntry))
---@field disconnect fun(self: self)

---@class TableTranslator
---@field name_space string
---@field lang_name string
---@field memorize_callback fun(ce: CommitEntry)
---@field enable_charset_filter boolean
---@field enable_encoder boolean
---@field enable_sentence boolean
---@field sentence_over_completion boolean
---@field encode_commit_history boolean
---@field max_phrase_length integer
---@field max_homographs integer
---@field delimiters string
---@field tag string
---@field enable_completion boolean
---@field contextual_suggestions boolean
---@field strict_spelling boolean
---@field initial_quality number
---@field preedit_formatter Projection
---@field comment_formatter Projection
---@field dict Dictionary
---@field user_dict UserDictionary
---@field translator Translator
---@field query fun(self: self, input: string, segment: Segment): Translation
---@field start_session fun(self: self): boolean
---@field finish_session fun(self: self): boolean
---@field discard_session fun(self: self): boolean
---@field memorize fun(self: self, callback: fun(ce: CommitEntry))
---@field update_entry fun(self: self, entry: DictEntry, commits: integer, prefix: string): boolean
---@field reload_user_dict_disabling_patterns fun(self: self, config_list: ConfigList): boolean
---@field set_memorize_callback fun(self: self, callback: fun(ce: CommitEntry))
---@field disconnect fun(self: self)

---@class Filter
---@field name_space string
---@field apply fun(self: self, translation: Translation): Translation

---@class Notifier
---@field connect fun(self: self, f: fun(ctx: Context), group: integer|nil): Connection

---@class OptionUpdateNotifier: Notifier
---@field connect fun(self: self, f: fun(ctx: Context, name: string), group:integer|nil): function[]

---@class PropertyUpdateNotifier: Notifier
---@field connect fun(self: self, f: fun(ctx: Context, name: string), group:integer|nil): function[]

---@class KeyEventNotifier: Notifier
---@field connect fun(self: self, f: fun(ctx: Context, key: string), group:integer|nil): function[]

---@class Connection
---@field disconnect fun(self: self)

---@class Switcher
---@field attached_engine Engine
---@field user_config Config
---@field active boolean
---@field process_key fun(self: self, key_event: KeyEvent): boolean
---@field select_next_schema fun(self: self)
---@field is_auto_save fun(self: self, option: string): boolean
---@field refresh_menu fun(self: self)
---@field activate fun(self: self)
---@field deactivate fun(self: self)

---@param engine Engine
---@return Switcher
function Switcher(engine) end

---@class CommitRecord
---@field text string
---@field type string

---@class CommitHistory
---@field size integer
---@field push fun(self: self, key_event: KeyEvent)
---@field back fun(self: self): CommitRecord|nil
---@field to_table fun(self: self): CommitRecord[]
---@field iter fun(self: self): fun(): (number, CommitRecord)|nil
---@field latest_text fun(self: self): string
---@field empty fun(self: self): boolean
---@field clear fun(self: self)
---@field pop_back fun(self: self)

---@class DbAccessor
---@field reset fun(self: self): boolean
---@field jump fun(self: self, prefix: string): boolean
---@field iter fun(self: self): fun(): (string, string) | nil

---@class UserDb
---@field _loaded boolean
---@field read_only boolean
---@field disabled boolean
---@field name string
---@field file_name string
---@field open fun(self: self): boolean
---@field open_read_only fun(self: self): boolean
---@field close fun(self: self): boolean
---@field query fun(self: self, prefix: string): DbAccessor
---@field fetch fun(self: self, key: string): string|nil
---@field update fun(self: self, key: string, value: string): boolean
---@field erase fun(self: self, key: string): boolean
---@field loaded fun(self: self): boolean
---@field disable fun(self: self): boolean
---@field enable fun(self: self): boolean

---@param db_name string
---@param db_class string
---@return UserDb
function UserDb(db_name, db_class) end

---@class LevelDb: UserDb

---@param db_name string
---@return LevelDb
function LevelDb(db_name) end

---@class TableDb: UserDb

---@param db_name string
---@return TableDb
function TableDb(db_name) end
