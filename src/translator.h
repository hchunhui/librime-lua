/*
 * translator.h
 * Copyright (C) 2021 Shewer Lu <shewer@gmail.com>
 *
 * Distributed under terms of the MIT license.
 *
 */

#ifndef __TRANSLATOR_H
#define __TRANSLATOR_H

#include <rime/gear/script_translator.h>
#include <rime/dict/corrector.h>   // StriptTranslator
#include <rime/gear/table_translator.h>
#include <rime/gear/unity_table_encoder.h> //table_translator
#include <rime/gear/poet.h>

#include "lua_gears.h"
#include "lib/lua_templates.h"

using namespace rime;

namespace TicketReg {
  typedef Ticket T;
  an<T> make(Engine  *e,const string &ns) {
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
    {"engine" ,WRAPMEM_GET(T::engine)},
    {"schema" ,WRAPMEM_GET(T::schema)},
    {"name_space" ,WRAPMEM_GET(T::name_space)},
    {"klass" ,WRAPMEM_GET(T::klass)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    //{"engine" ,WRAPMEM_SET(T::engine)},
    //{"schema" ,WRAPMEM_SET(T::schema)},
    {"name_space" ,WRAPMEM_SET(T::name_space)},
    //{"klass" ,WRAPMEM_SET(T::klass)},
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

namespace TranslatorReg {
  typedef Translator T;


  static const luaL_Reg funcs[] = {
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    {"query", WRAPMEM(T::Query)}, // translator.h_
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
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
  an<T> make(const Ticket & tt ){
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

  // gear/translator_common.h   TranslatorOption
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

  // gear/translator_common.h   TranslatorOption
  static const luaL_Reg vars_get[] = {
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };
}


#endif /* !TRANSLATOR_H */
