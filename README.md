# librime-lua: Extending [RIME](https://rime.im) with Lua scripts

[![Build status](https://ci.appveyor.com/api/projects/status/github/hchunhui/librime-lua?svg=true)](https://ci.appveyor.com/project/hchunhui/librime-lua)

Features
===
 - Supports extending RIME processors, segmentors, translators and filters
 - Provides high-level programming model for translators and filters
 - Loaded dynamically as a librime plugin

Usage
===
1. Create `PATH_TO_RIME_USER_DATA_DIR/rime.lua`:

    ```
    function date_translator(input, seg)
       if (input == "date") then
          --- Candidate(type, start, end, text, comment)
          yield(Candidate("date", seg.start, seg._end, os.date("%Y年%m月%d日"), " 日期"))
       end
    end
    
    function single_char_first_filter(input)
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
    ```

    More sample: [rime.lua](https://github.com/hchunhui/librime-lua/tree/master/sample/rime.lua)

    Documentation: TODO

2. Reference Lua functions in your schema:

    ```
    engine:
      ...
      translators:
        ...
        - lua_translator@date_translator
        - lua_translator@other_lua_function1
        ...
      filters:
        ...
        - lua_filter@single_char_first_filter
        - lua_filter@other_lua_function2
    ```

3. Deploy & try


Build
===

Build dependencies
---
  - librime >= 1.5.0
  - LuaJIT 2 / Lua 5.1 / Lua 5.2 / Lua 5.3 / Lua 5.4

Prebuilt versions
---
  - Windows
    - [1.4.0 backport](https://github.com/hchunhui/librime-lua/releases)
    - [master](https://ci.appveyor.com/project/hchunhui/librime-lua/build/artifacts)

Instructions
---
1. Prepare source code

   Move the source to the `plugins` directory of librime:
   ```
   mv librime-lua $PATH_TO_RIME_SOURCE/plugins/lua
   ```

   Or you can use the `install-plugins.sh` script to automatically fetch librime-lua:
   ```
   cd $PATH_TO_RIME_SOURCE
   bash install-plugins.sh hchunhui/librime-lua
   ```

2. Install dependencies

   Install development files of Lua:
   ```
   # For Debian/Ubuntu:
   sudo apt install liblua5.3-dev   # or libluajit-5.1-dev
   ```
   The build system will use `pkg-config` to search Lua.

   The build system also supports building Lua from source in the `thirdparty` directory.
   The `thirdparty` directory can be downloaded using the following commands:
   ```
   cd $PATH_TO_RIME_SOURCE/plugins/lua
   git clone https://github.com/hchunhui/librime-lua.git -b thirdparty --depth=1 thirdparty
   ```

3. Build

   Follow the librime's build instructions.
   ```
   # On Linux, merged build
   make merged-plugins
   sudo make install
   ```

   For more information on RIME plugins,
   see [here](https://github.com/rime/librime/tree/master/sample).
