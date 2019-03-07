# librime-lua: Extending [RIME](https://rime.im) with Lua scripts

Features
===
 - Supports extending RIME processors, segmentors, translators and filters
 - Provides high-level programming model for translators and filters
 - Loaded dynamically as a librime plugin

Usage
===
0. Build librime-lua:

    - Windows Prebuilt (merged build): https://ci.appveyor.com/api/buildjobs/c3bm0vgcg9ed36hu/artifacts/rime.zip

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
librime-lua has two build mode: merged and separated.
Now the former mode is recommended for compatibility.

The former mode merges the plugin code into one librime library.
To use the plugin, no modification to existing RIME frontends is required.
Just replace the librime library with the merged build.

The latter mode generates a new librime-lua shared library.
The library is used to dynamically loaded and configured by RIME frontends.
To use the plugin, some modification is required to existing RIME frontends.
In addition, separated build is not supported on Windows platform now.

Build dependencies
---
  - librime-master (>1.4.0)
  - LuaJIT 2 / Lua 5.2 / Lua 5.3

Merged Build
---
```
# Place the source in the plugins directory of librime
mv librime-lua PATH_TO_RIME_SOURCE/plugins

# Build & install librime
make
sudo make install
```

Separated Build
---
```
# Place the source in the plugins directory of librime
mv librime-lua PATH_TO_RIME_SOURCE/plugins

# Configure librime
mkdir -p build
cd build
cmake path_to_rime -DBUILD_MERGED_PLUGINS=OFF

# Make librime-lua
make rime-lua
```

The shared library `lib/librime-lua.so` is used to load dynamically.
