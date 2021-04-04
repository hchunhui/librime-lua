/*
 * config.cc
 * Copyright (C) 2021 Shewer Lu <shewer@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */


#include <rime/config.h>
#include <rime/config/config_data.h>
#include <rime/config/config_component.h>
#include <rime/config/config_types.h>
#include "lua_gears.h"
#include "lib/lua_templates.h"


//------------- Config  config

using namespace rime;

namespace ConfigValueReg {
  typedef ConfigValue T;
  typedef ConfigItem E;

  // an<T> make(){
  //  return New<T>();
  // };
  an<T> make(string s){
    return New<T>(s);
  };
#define QUOTE2(a,b) a.b
#define _ConfigValue_get( type_ , method )  \
  optional< type_ > get_ ##type_ (T &t) {\
      type_ v; \
	  if ( QUOTE2( t , method ) ( &v)) \
	    return v; \
	  return {} ; \
  }

_ConfigValue_get( bool , GetBool ) ;
_ConfigValue_get( int , GetInt) ;
_ConfigValue_get( double, GetDouble);
_ConfigValue_get( string, GetString);

#undef QUOTE2
#undef _ConfigValue_get 


/*
  optional<bool> get_bool(T &t) {
    bool v;
    if (t.GetBool( &v))
      return v;
      return {};
  }

  optional<int> get_int(T &t) {
    int v;
    if (t.GetInt( &v))
      return v;
    else
      return optional<int>{};
  }

  optional<double> get_double(T &t) {
    double v;
    if (t.GetDouble( &v))
      return v;
    else
      return optional<double>{};
      
  }

  optional<string> get_string(T &t) {
    string v;
    if (t.GetString( &v))
      return v;
    else
      return optional<string>{};
  }
*/

  bool set_string(T &t, const string &value) {
    return t.SetString( value);
  }

  string type(T &t){
    switch (t.type()) {
    case T::kNull: return "kNull";
    case T::kScalar: return "kScalar";
    case T::kList: return "kList";
    case T::kMap: return "kMap";
    }
    return "";
  }

  an<E> element(T &t){
	    return (an<E>) &t ;
  }

  static const luaL_Reg funcs[] = {
    {"ConfigValue", WRAP(make)},
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    {"get_bool",WRAP(get_bool)},
    {"get_int",WRAP(get_int)},
    {"get_double",WRAP(get_double)},
    {"set_bool", WRAPMEM(T::SetBool)},
	{"set_int", WRAPMEM(T::SetInt)},
	{"set_double", WRAPMEM(T::SetDouble)},
    {"get_string",WRAP(get_string)},
    {"set_string",WRAP(set_string)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    {"value",WRAP(get_string)},
    {"type",WRAP(type)},
	{"element",WRAP(element)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    {"value",WRAP(set_string)},
    { NULL, NULL },
  };
}
namespace ConfigListReg {
  typedef ConfigList T;
  typedef ConfigItem E;

  an<T> make(){
    return New<T>();
  };
  
  string type(T &t){
    switch (t.type()) {
    case T::kNull: return "kNull";
    case T::kScalar: return "kScalar";
    case T::kList: return "kList";
    case T::kMap: return "kMap";
    }
    return "";
  }

  an<E> element(T &t){
	    return (an<E>) &t ;
  }

  static const luaL_Reg funcs[] = {
    {"ConfigList", WRAP(make)},
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    {"get_at", WRAPMEM(T::GetAt)},
    {"get_value_at", WRAPMEM(T::GetValueAt)},
    {"set_at", WRAPMEM(T::SetAt)},
    {"append", WRAPMEM(T::Append)},
    {"insert", WRAPMEM(T::Insert)},
    {"clear", WRAPMEM(T::Clear)},
    {"empty", WRAPMEM(T::empty)},
    {"resize", WRAPMEM(T::Resize)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    {"size", WRAPMEM(T::size)},
    {"type",WRAP(type)},
	{"element",WRAP(element)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };
}


namespace ConfigMapReg {
  typedef ConfigMap T;
  typedef ConfigItem E;

  an<T> make(){
    return New<T>();
  }

  string type(T &t){
    switch (t.type()) {
    case T::kNull: return "kNull";
    case T::kScalar: return "kScalar";
    case T::kList: return "kList";
    case T::kMap: return "kMap";
    }
    return "";
  }

  size_t size(T &t){
    size_t count=0;
    for (auto it=t.begin(); it !=t.end();it++)
      count++ ;
    return count;
  }

  an<E> element(T &t){
    return (an<E>) &t ;
  }

  static const luaL_Reg funcs[] = {
    {"ConfigMap", WRAP(make)},
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    {"set", WRAPMEM(T::Set)},
    {"get", WRAPMEM(T::Get)},
    {"get_value", WRAPMEM(T::GetValue)},
    {"has_key", WRAPMEM(T::HasKey)},
    {"clear", WRAPMEM(T::Clear)},
    {"empty", WRAPMEM(T::empty)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    {"size", WRAP(size)},
    {"type",WRAP(type)},
    {"element",WRAP(element)},
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };
}

namespace ConfigItemReg {
  typedef ConfigItem T;

  string type(T &t){
    switch (t.type()) {
    case T::kNull: return "kNull";
    case T::kScalar: return "kScalar";
    case T::kList: return "kList";
    case T::kMap: return "kMap";
    }
    return "";
  }

//GET_CONFIG_START  
// test macro 
//sed -n -e '/^\/\/GET_CONFIG_START/,/^\/\/GET_CONFIG_END/p' plugins/lua/src/types.cc |gcc -E -
#define GET_CONFIG( name,obj, _t) \
  obj * name(T &t) {\
	if (t.type() == _t ) \
	   return ( obj*) (&t) ;\
	return nullptr; \
}

  GET_CONFIG(get_value,ConfigValue, T::kScalar);
  GET_CONFIG(get_list,ConfigList,T::kList);
  GET_CONFIG(get_map,ConfigMap, T::kMap);
#undef GET_CONFIG
//GET_CONFIG_START  
/*
  V* get_value(T &t){
    if (t.type() == T::kScalar)
      return (V*)(  &t);
    else
      return nullptr ;
  }

  L* get_list(T &t){
    if (t.type() == T::kList)
       return (L*) &t;
    else
      return nullptr ;
  }

  M* get_map(T &t){
    if (t.type() == T::kMap)
      return (M*) &t;
    else
      return nullptr ;
  }
*/
  static const luaL_Reg funcs[] = {
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    {"get_value",WRAP(get_value)},
    {"get_list",WRAP(get_list)},
    {"get_map",WRAP(get_map)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    {"type",WRAP(type)},
	{"empty",WRAPMEM(T::empty)},
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };
  
}


namespace ConfigReg {
  typedef Config T;
//GET_CONFIG_START  
// test macro 
//sed -n -e '/^\/\/GET_CONFIG_START/,/^\/\/GET_CONFIG_END/p' plugins/lua/src/types.cc |gcc -E -
#define QUOTE2( LSTR,RSTR) LSTR.RSTR
#define GET_CONFIG( DATA_TYPE , METHOD) \
  optional< DATA_TYPE > get_##DATA_TYPE (T &t, const string &path) {\
	  DATA_TYPE v;\
	  if ( QUOTE2(t,METHOD)(path, &v)  ) return v; \
	  return {};\
  }

GET_CONFIG(bool,GetBool);
GET_CONFIG(double,GetDouble);
GET_CONFIG(int,GetInt);
GET_CONFIG(string,GetString);

#undef QUOTE2
#undef GET_CONFIG
//GET_CONFIG_END

/*
  optional<bool> get_bool(T &t, const string &path) {
    bool v;
    if (t.GetBool(path, &v))
      return v;
    else
      return {};
  }

  optional<int> get_int(T &t, const string &path) {
    int v;
    if (t.GetInt(path, &v))
      return v;
    else
      return optional<int>{};
  }

  optional<double> get_double(T &t, const string &path) {
    double v;
    if (t.GetDouble(path, &v))
      return v;
    else
      return optional<double>{};
  }

  optional<string> get_string(T &t, const string &path) {
    string v;
    if (t.GetString(path, &v))
      return v;
    else
      return optional<string>{};
  }
*/

  // GetItem SetItem : overload function

  an<ConfigItem> get_item(T &t, const string & path){
    return t.GetItem(path);
  }
  
  // SetString : overload function
  bool set_string(T &t, const string &path, const string &value) {
    return t.SetString(path, value);
  }

//SET_CONFIG_START  
// test macro 
//sed -n -e '/^\/\/SET_CONFIG_START/,/^\/\/SET_CONFIG_END/p' plugins/lua/src/types.cc |gcc -E -
#define SET_CONFIG(func_name, obj) \
  bool func_name(T &t, const string &path,  obj  value){ \
	  return t.SetItem(path,value) ; \
  }

  SET_CONFIG(set_item, an<ConfigItem> );
  SET_CONFIG(set_value, an<ConfigValue> );
  SET_CONFIG(set_list, an<ConfigList> );
  SET_CONFIG(set_map, an<ConfigMap> );
#undef SET_CONFIG
//SET_CONFIG_END
  /*
  bool set_item(T &t ,const string &path, an<ConfigItem> item){
    return t.SetItem(path,item);
  }

  bool set_value(T &t, const string &path,  an<ConfigValue>  value) {
    return t.SetItem(path, value);
  }

  bool set_list(T &t, const string &path,  an<ConfigList>  value) {
    return t.SetItem(path, value);
  }

  bool set_map(T &t, const string &path, an<ConfigMap> value) {
    return t.SetItem(path, value);
  }
  */

  static const luaL_Reg funcs[] = {
    { NULL, NULL },
  };

  static const luaL_Reg methods[] = {
    //bool LoadFromStream(std::istream& stream);
    //bool SaveToStream(std::ostream& stream);
    { "load_from_file", WRAPMEM(T::LoadFromFile) },
    { "save_to_file", WRAPMEM(T::SaveToFile) },

    { "is_null", WRAPMEM(T::IsNull) },
    { "is_value", WRAPMEM(T::IsValue) },
    { "is_list", WRAPMEM(T::IsList) },
    { "is_map", WRAPMEM(T::IsMap) },

    { "get_bool", WRAP(get_bool) },
    { "get_int", WRAP(get_int) },
    { "get_double", WRAP(get_double) },
    { "get_string", WRAP(get_string) },

    { "set_bool", WRAPMEM(T::SetBool) },
    { "set_int", WRAPMEM(T::SetInt) },
    { "set_double", WRAPMEM(T::SetDouble) },
    { "set_string", WRAP(set_string) }, // redefine overload function 

    { "get_item", WRAP(get_item) }, // redefine overload function 
    { "set_item", WRAP(set_item) }, // create new function 

    //an<ConfigItem> GetItem(const string& path);
    //an<ConfigValue> GetValue(const string& path);
    //RIME_API an<ConfigList> GetList(const string& path);
    //RIME_API an<ConfigMap> GetMap(const string& path);

    { "get_value", WRAPMEM(T::GetValue) },
    { "get_list", WRAPMEM(T::GetList) },
    { "get_map", WRAPMEM(T::GetMap) },

    { "set_value", WRAP(set_value) }, // create new function 
    { "set_list", WRAP(set_list) }, // create new function 
    { "set_map", WRAP(set_map)}, // create new function 

    { "get_list_size", WRAPMEM(T::GetListSize) },

    //RIME_API bool SetItem(const string& path, an<ConfigItem> item);
    { NULL, NULL },
  };

  static const luaL_Reg vars_get[] = {
    { NULL, NULL },
  };

  static const luaL_Reg vars_set[] = {
    { NULL, NULL },
  };
}
