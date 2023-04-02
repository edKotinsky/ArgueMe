#ifndef ARGUEME_HPP
#define ARGUEME_HPP

#include "argfwd.hpp"
#include <cstdlib>

namespace arg {

  namespace utility {

    template <typename T>
    T from_string(std::string_view s) {
      if constexpr (std::is_same_v<T, std::string> ||
                    std::is_same_v<T, std::string_view>) {
        return std::string { s };
      } else {
        static_assert(has_operator_extraction<T>::value,
                      "Type must have defined operator<<");
        std::istringstream is(std::string { s });
        T res;
        is >> std::noskipws >> res;
        if (is.fail() || is.peek() != EOF)
          throw command_line_error("Cannot convert a string to a value", s);
        return res;
      }
    }

  } // namespace utility

  template <typename T>
  T const& utility::argument_template<T>::get() const noexcept {
    return value;
  }

  template <typename T>
  void value_argument<T>::parse(utility::command_line_impl& cmdline) {
    if (activited) throw command_line_error("Option can be appeared only once");
    activited = true;
    auto s = cmdline.next_argument();
    if (!s) throw command_line_error("Option requires a value");
    this->value = utility::from_string<T>(*s);
  }

  template <typename T>
  void multi_argument<T>::parse(utility::command_line_impl& cmdline) {
    auto s = cmdline.next_argument();
    if (!s) throw command_line_error("Option requires a value");
    T value = utility::from_string<T>(s);
    this->value.push_back(value);
  }

  template <typename T>
  void positional_argument<T>::parse(utility::command_line_impl& cmdline) {
    auto s = cmdline.next_argument();
    if (!s) throw command_line_error("Option requires a value");
    T value = utility::from_string<T>(s);
    this->value.push_back(value);
  }

} // namespace arg

#endif
