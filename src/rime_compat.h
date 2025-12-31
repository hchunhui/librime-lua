#ifndef RIME_COMPAT_H_
#define RIME_COMPAT_H_

#include <string>
#include <type_traits>
#include <rime/common.h>
#include <rime/service.h>
#include <rime_api.h>

namespace rime {

template<typename> using void_t = void;

template<typename T, typename = void>
struct COMPAT {
  static std::string get_shared_data_dir() {
    return std::string(rime_get_api()->get_shared_data_dir());
  }

  static std::string get_user_data_dir() {
    return std::string(rime_get_api()->get_user_data_dir());
  }
};

template<typename T>
struct COMPAT<T, void_t<decltype(std::declval<T>().user_data_dir.string())>> {
  static std::string get_shared_data_dir() {
    // path::string() returns native encoding on Windows
    T &deployer = rime::Service::instance().deployer();
    return deployer.shared_data_dir.string();
  }

  static std::string get_user_data_dir() {
    T &deployer = rime::Service::instance().deployer();
    return deployer.user_data_dir.string();
  }
};

} // namespace rime

#endif // RIME_COMPAT_H_
