/*
 * component.h
 * Copyright (C) 2021 Shewer Lu <shewer@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __COMPONENT_H
#define __COMPONENT_H
#include <rime/candidate.h>
#include <rime/translation.h>
#include <rime/gear/translator_commons.h>

#include <rime/dict/corrector.h>   // StriptTranslator 
#include <rime/gear/script_translator.h>
#include <rime/gear/table_translator.h>

#include <rime/gear/unity_table_encoder.h>
#include <rime/gear/memory.h>
#include <rime/gear/poet.h>




#include <rime/component.h>
#include <rime_api.h>
#include <rime/common.h>


#include "lua_gears.h"
#include "lib/lua_templates.h"

using namespace rime;

namespace TicketReg{
  typedef Ticket T;
    an<T> make(Engine * e,const string ns) {
		return New <T>(e,ns);
	}

  static const luaL_Reg funcs[] = {
	{"Ticket",WRAP(make) },
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
	//{"engine" ,WRAPMEM(T::engine)},  
	//{"schema" ,WRAPMEM(T::schema)},  
	//{"name_space" ,WRAPMEM(T::name_space)},  
	//{"klass" ,WRAPMEM(T::klass)},  
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
	//{"engine" ,WRAPMEM(T::engine)},  
	//{"schema" ,WRAPMEM(T::schema)},  
	//{"name_space" ,WRAPMEM(T::name_space)},  
	//{"klass" ,WRAPMEM(T::klass)},  
    { NULL, NULL },
  };

}
namespace ComponentReg {

  typedef Translator T;
  typedef TableTranslator TT;
  typedef ScriptTranslator ST;

  typedef Processor P;
  typedef Filter F;
  typedef Dictionary D;
  an<T> Translator(const string component_name ,Engine * e, string ns ){
	  if ( auto c = T::Require(component_name) ) {
		  Ticket t= Ticket(e,ns) ; 
//		  an <T> tran;
		  return (an<T>)  c->Create(t);
	  }
	  return {};

  }

				  

  static const luaL_Reg funcs[] = {
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };

}

namespace UnionTranslationReg{
  typedef UnionTranslation T;
  typedef Translation S;

  an<T> make() {
    return (an<T>) New <T>();
  }
  void  append( T &t, an<S> s) {
	t += s;
  }
  bool exhausted(T &t) {
	  return t.exhausted();
  }
  an<S> get_translation(T &t){
	  return (an<S>)  &t ;
  }
  optional<an<Candidate>> next(T &t) {
    if (t.exhausted())
      return {};

    auto c = t.Peek();
    t.Next();
    return c;
  }


  int raw_iter(lua_State *L) {
    lua_pushcfunction(L, WRAP(next));
    lua_pushvalue(L, 1);
    return 2;
  }


  static const luaL_Reg funcs[] = {
    {"UnionTranslation",WRAP(make)},  
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
	{"next",WRAP(next)},
	{"iter",raw_iter},
	{"append", WRAP(append)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
	{ "exhausted", WRAP(exhausted)},
	{ "translation", WRAP(get_translation)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };
}
namespace FifoTranslationReg{
  typedef FifoTranslation T;
  typedef Translation S;

  an<T> make() {
    return (an<T>) New <T>();
  }

  typedef an<Candidate>  R;
  //typedef optional<an<Candidate>> R;

  optional<an<Candidate>> next(T &t) {
    if (t.exhausted())
      return {};

    auto c = t.Peek();
    t.Next();
    return c;
  }

  int raw_iter(lua_State *L) {
    lua_pushcfunction(L, WRAP(next));
    lua_pushvalue(L, 1);
    return 2;
  }

  bool exhausted(T &t) {
	  return t.exhausted();
  }
  an<S> get_translation(T &t){
	  return (an<S>)  &t ;
  }




  static const luaL_Reg funcs[] = {
    {"FifoTranslation",WRAP(make)},  
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
	{"iter",raw_iter},
	{"append", WRAPMEM(T::Append)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
	{ "exhausted", WRAP(exhausted)},
	{ "translation", WRAP(get_translation)},
	{ "size", WRAPMEM(T::size)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };
}
namespace MergedTranslationReg{
  typedef MergedTranslation T;
  typedef Translation S;
  an<T> make() {

	const CandidateList c;
	//= New CandidateList;
    return (an<T>) New <T>(c);
  }
  void  append( T &t, an<S> s) {
	t += s;
  }
  bool exhausted(T &t) {
	  return t.exhausted();
  }
  an<S> get_translation(T &t){
	  return (an<S>)  &t ;
  }
  optional<an<Candidate>> next(T &t) {
    if (t.exhausted())
      return {};

    auto c = t.Peek();
    t.Next();
    return c;
  }


  int raw_iter(lua_State *L) {
    lua_pushcfunction(L, WRAP(next));
    lua_pushvalue(L, 1);
    return 2;
  }


  static const luaL_Reg funcs[] = {
    {"MergedTranslation",WRAP(make)},  
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
	{"next",WRAP(next)},
	{"iter",raw_iter},
	{"append", WRAP(append)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
	{ "exhausted", WRAP(exhausted)},
	{ "translation", WRAP(get_translation)},
	{ "size", WRAPMEM(T::size)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };
}
/*
namespace TempReg {
  typedef Switcher T;

  an<T> make(Engine *engine) {
    return New<T>(engine);
  }

  static const luaL_Reg funcs[] = {
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };
}

*/





#endif /* !COMPONENT_H */
