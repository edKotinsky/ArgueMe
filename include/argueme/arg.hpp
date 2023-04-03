#ifndef ARGUEMEFWD_HPP
#define ARGUEMEFWD_HPP

#include <exception>
#include <functional>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace arg {

  class argument_error : public std::exception {
  public:
    template <class String>
    argument_error(String&& what) : what_str(std::move(what)), arg_name() {}

    template <class String1, class String2>
    argument_error(String1&& what, String2&& argname)
        : what_str(std::move(what)), arg_name(std::move(argname)) {}

    virtual char const* what() const noexcept override {
      return what_str.c_str();
    }

    char const* argname() const noexcept { return arg_name.c_str(); }
  private:
    std::string what_str;
    std::string arg_name;
  };

  class command_line_error : public std::exception {
  public:
    command_line_error(std::string what) : what_str(what), info_str() {}

    command_line_error(std::string what, std::string info)
        : what_str(what), info_str(info) {}

    command_line_error(std::string_view what, std::string_view info)
        : what_str(what), info_str(info) {}

    virtual const char* what() const noexcept { return what_str.c_str(); }

    std::string const& info() const noexcept { return info_str; }
  private:
    std::string what_str;
    std::string info_str;
  };

  namespace utility {

    template <typename T>
    class argument_template {
    public:
      argument_template(T default_value) : value(default_value) {}

      T const& get() const noexcept;
    protected:
      T value;
    };

    class command_line_impl;

    class argument {
    public:
      /*
       * Parses a vector of arguments.
       *
       * Takes a command line private API instance. That instance allows to
       * iterate over a vector and check, if the string is an argument.
       */
      virtual void parse(command_line_impl& cmdline) = 0;

      virtual ~argument() {}
    };

    class command_line_impl {
    public:
      using svvec_t = std::vector<std::string_view>;

      command_line_impl(std::string_view longname_start,
                        std::string_view shortname_start) {
        lname_prefix = longname_start;
        sname_prefix = shortname_start;
      }

      /*
       * Parses a vector of arguments.
       *
       * In a `while` loop goes through the vector, and tries to find each
       * string in an `args` dictionary. If the current string is found, i.e it
       * is a named argument, then calls its `parse` method.
       *
       * Otherwise, if current string is not found, checks if there are
       * positional arguments. If so, assigns current string to a positional
       * argument and increments a counter of positional arguments.
       *
       * If the string is not found in a dictionary, and a count of positional
       * arguments == `positional_args.size()`, then throws an error.
       *
       * May throw exception of type `argument_error`;
       *
       * If current positional arg iterator != positional args vector's end and
       * there is a least one mandatory argument remained, then throws exception
       * `argument_error`.
       */
      void parse(svvec_t const& input_vec);

      /*
       * Checks if `s` is an argument.
       *
       * Removes a prefix, if it is found in `s`, and finds the resulting string
       * in an args dictionary. If an argument is found, then `s` is an argument
       * name.
       */
      bool is_argument(std::string_view s);

      /*
       * Checks, if `s` starts with longname or shortname prefix. If so, returns
       * `std::string_view` without these first characters. Otherwise, returns
       * `s` itself.
       */
      std::string_view remove_prefix(std::string_view s);

      /*
       * Checks if `str` starts with `subs`.
       */
      static constexpr bool starts_with(std::string_view str,
                                        std::string_view subs);

      /*
       * Increments an input vector iterator and returns optional of the next
       * argument. If an end of input vector is reached, then assigns to an
       * input vector's pointer a null value. In the case if input vector is
       * null, returns empty optional.
       */
      std::optional<std::string_view> next_argument();

      /*
       * Returns optional of the current argument's string_view. If an end of
       * input vector is reached or input vector is null, then returns an empty
       * optional.
       */
      std::optional<std::string_view> get_argument();

      /*
       * Attaches named argument
       */
      void attach_argument(std::string_view lname, std::string_view sname,
                           utility::argument& arg);

      /*
       * Attaches positional argument. Positional arguments have no names and
       * they are identified only by its position in the vector `p_args`.
       */
      void attach_argument(utility::argument& arg, bool arg_mandatory);

    private:
      struct posarg_wrapper {
      public:
        posarg_wrapper(argument& arg, bool mandatory) noexcept
            : arg(arg), mandatory(mandatory) {}

        inline argument& get() const noexcept { return arg.get(); }

        inline bool is_mandatory() const noexcept { return mandatory; }

      private:
        std::reference_wrapper<argument> arg;
        bool const mandatory;
      };

      using argument_t = std::reference_wrapper<argument>;
      using argsvec_t = std::vector<argument_t>;

      svvec_t const* input;
      typename svvec_t::const_iterator current;

      using pargsvec_t = std::vector<posarg_wrapper>;
      pargsvec_t p_args;
      typename pargsvec_t::const_iterator cur_pos_arg;
      std::unordered_map<std::string_view, argument_t> args;

      std::string_view lname_prefix;
      std::string_view sname_prefix;
    };

    template <class C>
    struct has_operator_extraction_impl {
    private:
      template <class T, std::istream& (std::istream::*) (T&) =
                             (&std::istream::operator>>)>
      struct wrapper {};

      template <class T>
      static std::true_type check(wrapper<C>*);

      template <class T>
      static std::false_type check(...);
    public:
      static constexpr bool value =
          std::is_same_v<decltype(check<C>(0)), std::true_type>;
    };

    template <class C>
    struct has_operator_extraction
        : std::bool_constant<has_operator_extraction_impl<C>::value> {};

    template <typename T>
    T from_string(std::string_view s);

  } // namespace utility

  class command_line {
  public:
    command_line(std::string_view longname_prefix,
                 std::string_view shortname_prefix)
        : impl(longname_prefix, shortname_prefix) {}

    /*
     * Attaches a named argument. Intended for internal usage, shall be called
     * only by arguments.
     */
    void attach(std::string_view longname, std::string_view shortname,
                utility::argument& arg);

    /*
     * Attaches a positional argument. Intended for internal usage, shall be
     * called only by arguments.
     */
    void attach(utility::argument& arg, bool mandatory);

    void parse(char const** argv, int argc);
    void parse(std::vector<std::string_view> const& vec);
    void parse(std::vector<std::string> const& vec);
  private:
    utility::command_line_impl impl;
  };

  template <typename T>
  class value_argument : public utility::argument,
                         public utility::argument_template<T> {
  public:
    value_argument(std::string_view longname, std::string_view shortname,
                   command_line& cmdline, T default_value = T {})
        : utility::argument_template<T>(default_value) {
      cmdline.attach(longname, shortname, *this);
    }

    virtual void parse(utility::command_line_impl& cmdline) override final {
      if (activited) throw argument_error("Option can be appeared only once");
      activited = true;
      auto s = cmdline.next_argument();
      if (!s) throw argument_error("Option requires a value");
      this->value = utility::from_string<T>(*s);
    }

    virtual ~value_argument() override {}
  private:
    bool activited = false;
  };

  template <typename T>
  class multi_argument : public utility::argument {
  public:
    multi_argument(std::string_view longname, std::string_view shortname,
                   command_line& cmdline) {
      cmdline.attach(longname, shortname, *this);
    }

    virtual void parse(utility::command_line_impl& cmdline) override final {
      auto s = cmdline.next_argument();
      if (!s) throw argument_error("Option requires a value");
      T value = utility::from_string<T>(*s);
      this->value.push_back(value);
    }

    virtual ~multi_argument() override {};

    std::vector<T> const& get() const noexcept { return value; }
  private:
    std::vector<T> value;
  };

  template <typename T>
  class positional_argument : public utility::argument,
                              public utility::argument_template<T> {
  public:
    positional_argument(command_line& cmdline, bool is_mandatory = false,
                        T default_value = T {})
        : utility::argument_template<T>(default_value) {
      cmdline.attach(*this, is_mandatory);
    }

    virtual void parse(utility::command_line_impl& cmdline) override final {
      auto s = cmdline.get_argument();
      if (!s) throw argument_error("Option requires a value");
      this->value = utility::from_string<T>(*s);
    }

    virtual ~positional_argument() override {}
  };

  class switch_argument : public utility::argument,
                          public utility::argument_template<bool> {
  public:
    switch_argument(std::string_view longname, std::string_view shortname,
                    command_line& cmdline, bool default_value = false)
        : utility::argument_template<bool>(default_value) {
      value = default_value;
      cmdline.attach(longname, shortname, *this);
    }

    virtual void parse(utility::command_line_impl&) override final;
    virtual ~switch_argument() override;
  };

  /*
   * Definitions
   */

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
        if (is.fail() || is.peek() != EOF) {
          std::string msg { "Cannot convert a string `" };
          msg.append(s);
          msg.append("` to a value");
          throw argument_error(msg);
        }
        return res;
      }
    }

  } // namespace utility

  template <typename T>
  T const& utility::argument_template<T>::get() const noexcept {
    return value;
  }

} // namespace arg

#endif
