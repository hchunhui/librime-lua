/*
 * translator.h
 * Copyright (C) 2021 Shewer Lu <shewer@gmail.com>
 *
 * Distributed under terms of the MIT license.
 *
 */

#ifndef TRANSLATOR_H
#define TRANSLATOR_H




#include <rime/dict/corrector.h>   // StriptTranslator 
#include <rime/gear/table_translator.h>

#include <rime/gear/script_translator.h>
#include "lua_gears.h"
#include "lib/lua_templates.h"
using namespace rime;

namespace TranslatorReg {
	typedef Translator T ;


  static const luaL_Reg funcs[] = {
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
	{"query",WRAPMEM(T::Query)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
	//{"enable_completion",WRAPMEM(TableTranslator::enable_completion)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
	//{"enable_completion",WRAPMEM(TableTranslator::set_enable_completion)},
    { NULL, NULL },
  };
}
namespace TranslatorOptionsReg {
  typedef TranslatorOptions T;

  static const luaL_Reg funcs[] = {
  	//{"TableTranslator",WRAP(make)},
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
	{"tag",WRAPMEM(T::tag)}, // string
	{"enable_completion",WRAPMEM(T::enable_completion)}, // bool
	{"strict_spellijng",WRAPMEM(T::strict_spelling)}, // bool
	{"initial_quality",WRAPMEM(T::initial_quality)}, // bool 
	{"contextual_suggestions",WRAPMEM(T::contextual_suggestions)}, // bool
	{"preedit_formatter",WRAPMEM(T::preedit_formatter)},  //  Projection& 
	{"comment_formatter",WRAPMEM(T::comment_formatter)},  //  Projection&

	
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
	{"tag",WRAPMEM(T::set_tag)},
	{"enable_completion",WRAPMEM(T::set_enable_completion)},
	{"strict_spelling",WRAPMEM(T::set_strict_spelling)},
	{"initial_quality",WRAPMEM(T::set_initial_quality)},
	{"contextual_suggestions",WRAPMEM(T::set_contextual_suggestions)},
    { NULL, NULL },
  };
}


namespace MemoryReg {
  typedef Memory T;
/*
  an<Dictionary>  get_option(T &t) {
	  return (an<Dictionary>) &t ;
  }
  an<UserDictionary>  get_option(T &t) {
	  return (an<UserDictionary>) &t ;
  }
*/	

  static const luaL_Reg funcs[] = {
  	//{"TableTranslator",WRAP(make)},
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
	//gear/memory.h
	{"dict",WRAPMEM(T::dict)}, // Dictionary *
	{"user_dict",WRAPMEM(T::user_dict)}, // UserDictionary *
	{"language",WRAPMEM(T::language)}, // const Language *
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };
}

namespace TableTranslatorReg {
  typedef TableTranslator T;
  typedef Translator TT;
  typedef Memory M;
  typedef TranslatorOptions TO;

 // typedef Ticket TT
  an<T> make( Ticket & tt ){
	  return New <T>(tt);
  }
  an<TT> get_translator(T &t) {
	  return (an<TT>) &t;
  }
  an<M>  get_memory(T &t) {
	  return (an<M>) &t ;
  }
  an<TO>  get_option(T &t) {
	  return (an<TO>) &t ;
  }


  static const luaL_Reg funcs[] = {
  	{"TableTranslator",WRAP(make)},
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
	{"query", WRAPMEM(T::Query)}, // translator.h_
    { NULL, NULL },
  };

	// gear/translator_common.h   TranslatorOptions 
  static const luaL_Reg vars_get[] = {
	{"translator",WRAP(get_translator)},
	{"memory",WRAP(get_memory)},
	{"option", WRAP(get_option)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };
}



namespace ScriptTranslatorReg {
  typedef ScriptTranslator T;
  typedef Translator TT;
  typedef Memory M;
  typedef TranslatorOptions TO;

 // typedef Ticket TT
  an<T> make( Ticket & tt ){
	  return New <T>(tt);
  }
  an<TT> get_translator(T &t) {
	  return (an<TT>) &t;
  }
  an<M>  get_memory(T &t) {
	  return (an<M>) &t ;
  }
  an<TO>  get_option(T &t) {
	  return (an<TO>) &t ;
  }


  static const luaL_Reg funcs[] = {
  	{"ScriptTranslator",WRAP(make)},
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
	{"query", WRAPMEM(T::Query)}, // translator.h_
    { NULL, NULL },
  };

	// gear/translator_common.h   TranslatorOptions 
  static const luaL_Reg vars_get[] = {
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };
}


#endif /* !TRANSLATOR_H */
