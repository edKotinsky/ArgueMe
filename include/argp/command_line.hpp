#ifndef ARGP_COMMAND_LINE_HPP
#define ARGP_COMMAND_LINE_HPP

#include "command_line_fwd.hpp"

namespace argp {

  namespace utility {

    template <typename T>
    T from_string(std::string_view s) {
      if constexpr (std::is_same_v<T, std::string> ||
                    std::is_same_v<T, std::string_view>) {
        return s;
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

} // namespace argp

#endif
