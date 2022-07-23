#pragma once

#ifdef LIBBASE_ENABLE_TRACING

#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace base {
namespace detail {

class ArgumentPacker {
 public:
  using StringArguments = std::vector<std::pair<std::string, std::string>>;
  using IntegerArguments = std::vector<std::pair<std::string, uint64_t>>;

  template <typename... Ts>
  static StringArguments PackStringArguments(Ts&&... args) {
    static_assert(
        sizeof...(args) % 2 == 0,
        "PackStringArguments: arguments must be provided in key-value pairs");
    StringArguments result;

    result.reserve(sizeof...(args) / 2);
    AppendArguments(result, std::forward<Ts>(args)...);

    return result;
  }

  template <typename... Ts>
  static IntegerArguments PackIntegerArguments(Ts&&... args) {
    static_assert(
        sizeof...(args) % 2 == 0,
        "PackIntegerArguments: arguments must be provided in key-value pairs!");
    IntegerArguments result;

    result.reserve(sizeof...(args) / 2);
    AppendArguments(result, std::forward<Ts>(args)...);

    return result;
  }

 private:
  template <typename C>
  static void AppendArguments(C&) {}

  template <typename C, typename K, typename V, typename... Ts>
  static void AppendArguments(C& container, K&& key, V&& value, Ts&&... rest) {
    container.emplace_back(std::piecewise_construct, std::forward_as_tuple(key),
                           std::forward_as_tuple(value));
    AppendArguments(container, std::forward<Ts>(rest)...);
  }
};

}  // namespace detail
}  // namespace base

#endif
