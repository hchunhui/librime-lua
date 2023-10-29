#if __cplusplus >= 201703L || _MSVC_LANG >= 201703L
#include "lib/luatype_std_optional.h"
using std::optional;
#else
#include "lib/luatype_boost_optional.h"
using boost::optional;
#endif
