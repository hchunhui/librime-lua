#! /usr/bin/env lua
--
-- test_translator.lua
-- Copyright (C) 2023 Shewer Lu <shewer@gmail.com>
--
-- Distributed under terms of the MIT license.
--

local lu = require 'tools/luaunit'
local err_fmt="%s:%s %s"
local M = {}

function M:Setup()
  self.tran1=Component.Translator(env.engine,"translator","table_translator")
  self.tran2=Component.Translator(env.engine,"translator","script_translator")
  self.mem1= Memory(env.engine,Schema('cangjie5'),"translator",'table_translator')
  --os.remove(self.fname)
end
function M:tearDown()
  self.tran1=nil
  self.tran2=nil
  self.mem1=nil
  --os.remove(self.fname)
end

function M:test_create_translator()
  local t1= Ticket(env.engine, "", "table_translator@translator")
  local t2= Ticket(env.engine, "translator", "table_translator")
  local t3= Ticket(env.engine.schema,"translator")

  local args= {
    {t1},
    {t2},
    {t3},
    --{t4},
    {env.engien,"translator", "table_translator"},
    {env.engien,"", "table_translator@translator"},
    {env.engien,env.schema,"translator", "table_translator"},
    {env.engien,env.schema,"", "table_translator@translator"},
    {env.engien,Schema('lua_test'),"translator", "table_translator"},
    {env.engien,Schema('lua_test'),"", "table_translator@translator"},
  }


end

function M:test_instance()
  local tran1_str="table_tran"
  for i , v in next, {self.tran1,self.tran2,self.mem1} do
    lu.assertNotNil(v, err_fmt:format(i,v,"not exist Component.Translator of ") )
    lu.assertNotNil(self.tran1.options, err_fmt:format(self.tran1, "" ,"not exist options from" ))

    lu.assertNotNil(v.language, err_fmt:format(i,v,"not exist language from ") )
    --lu.assertNotNil(v.language_name, err_fmt:format(i,v,"not exist language_name from " ) )
    --lu.assertIsString(v.language_name, err_fmt:format(i,v, "type(string) error language_name ") )
    --lu.assertEquals(v.language_name,"cangjie5", err_fmt:format(i,v, "language_name not match cangjie5" ) )

    -- test obj.memory
    local vm = v.memory
    lu.assertNotNil(vm, err_fmt:format(i,vv,"not exist Component.Translator.memory of ") )
    lu.assertNotNil(vm.language, err_fmt:format(i,vv,"not exist language from ") )
    lu.assertNotNil(vm.language_name, err_fmt:format(i,vv,"not exist language_name from " ) )
    lu.assertIsString(vm.language_name, err_fmt:format(i,vv, "type(string) error language_name ") )
    lu.assertEquals(vm.language_name,"cangjie5", err_fmt:format(i,v, "language_name not match cangjie5" ) )

    lu.assertNotNil(vm.dict , err_fmt:format(i,v, "not exist dict") )
    lu.assertNotNil(vm.user_dict , err_fmt:format(i,v, "not exist user_dict") )
  end

  lu.assertNotNil(self.tran1.table_options, err_fmt:format(self.tran1,"","not exist table_options from") )
  lu.assertNil(self.tran1.script_options, err_fmt:format(self.tran1, "","script_options must nil from ") )

  lu.assertNil(self.tran2.table_options, err_fmt:format(self.tran2,"","table_options must nil from") )
  lu.assertNotNil(self.tran2.script_options, err_fmt:format(self.tran2, "","not exist script_options from ") )
end
function M:test_luamemorydict()
  local dict = self.mem1.dict
  self.mem1:dict_lookup('ab',true, 0)

  local tab={}
  for entry in self.mem1:iter_dict() do
    table.insert(tab,entry)
  end
  lu.assertTrue( #tab > 0, "entry not found from dict:lookup_words()")
  lu.assertEquals(tab[1].text, "明")

  print("---Phrase() and dictentry:to_phrase() test")
  local tran = { self.tran1, self.tran2, self.tran1.memory, self.tran2.memory, self.mem1 , self.mem1.memory}
  for i,v in next, tran do
    --print(i,v)
    local ph = Phrase(v,'test', 0, 2,tab[1])
    lu.assertNotNil( ph ,("%s:%s %s"):format(i,v,"obj.language can't accept"))
    local ph1 = tab[1]:to_phrase(v, "test" , 0,2)
    lu.assertNotNil( ph1 ,("%s:%s %s"):format(i,v,"dictentry:to_phrase( obj.language) can't accept"))
  end
end

function M:test_dict()
  local dict = self.tran1.memory.dict
  local tab={}
  for entry in dict:lookup_words('ab',true, 0):iter() do
    table.insert(tab,entry)
  end
  lu.assertTrue( #tab > 0, "entry not found from dict:lookup_words()")
  lu.assertEquals(tab[1].text, "明")

  print("---Phrase() and dictentry:to_phrase() test")
  local tran = { self.tran1, self.tran2, self.tran1.memory, self.tran2.memory, self.mem1 , self.mem1.memory}
  for i,v in next, tran do
    --print(i,v)
    local ph = Phrase(v,'test', 0, 2,tab[1])
    --print(i,helper(ph))
    lu.assertNotNil( ph ,("%s:%s %s"):format(i,v,"obj.language can't accept"))
    local ph1 = tab[1]:to_phrase(v, "test" , 0,2)
    --print(i,helper(ph))
    lu.assertNotNil( ph1 ,("%s:%s %s"):format(i,v,"dictentry:to_phrase( obj.language) can't accept"))
  end
end
return M

