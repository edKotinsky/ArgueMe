#ifndef ARGP_PARSER_HPP
#define ARGP_PARSER_HPP

#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace argp {

  namespace __details {
    template <typename T>
    class argument_template {
    public:
      argument_template(T default_value = T(0)) : value(default_value) {}

      void set(T value);
      T const& get() const noexcept;
    private:
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
      virtual ~argument() = 0;
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
                           __details::argument& arg);

      /*
       * Attaches positional argument. Positional arguments have no names and
       * they are identified only by its position in the vector `p_args`.
       */
      void attach_argument(__details::argument& arg);

    private:
      using argument_t = std::reference_wrapper<argument>;
      using argsvec_t = std::vector<argument_t>;

      svvec_t const* input;
      typename svvec_t::const_iterator current;

      argsvec_t p_args;
      typename argsvec_t::const_iterator cur_pos_arg;
      std::unordered_map<std::string_view, argument_t> args;

      std::string_view lname_prefix;
      std::string_view sname_prefix;
    };

  } // namespace __details

  class command_line_error : public std::exception {
  public:
    command_line_error(std::string what) : what_str(what), info_str() {}

    command_line_error(std::string what, std::string info)
        : what_str(what), info_str(info) {}

    command_line_error(std::string_view what, std::string_view info)
        : what_str(what), info_str(info) {}

    virtual const char* what() const noexcept { return what_str.c_str(); }

    const char* info() const noexcept { return info_str.c_str(); }
  private:
    std::string what_str;
    std::string info_str;
  };

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
                __details::argument& arg);

    /*
     * Attaches a positional argument. Intended for internal usage, shall be
     * called only by arguments.
     */
    void attach(__details::argument& arg);

    void parse(char const** argv, int argc);
    void parse(std::vector<std::string_view> const& vec);
  private:
    __details::command_line_impl impl;
  };

  template <typename T>
  class value_argument : public __details::argument,
                         public __details::argument_template<T> {
  public:
    value_argument(std::string_view longname, std::string_view shortname,
                   command_line& cmdline, T default_value = T { 0 }) {
      value = default_value;
      cmdline.attach(longname, shortname, *this);
    }

    virtual void parse(__details::command_line_impl&) override final {}

    virtual ~value_argument() override final;
  private:
    T value;
    bool activited = false;
  };

  template <typename T>
  class multi_argument : public __details::argument,
                         public __details::argument_template<T> {
  public:
    multi_argument(std::string_view longname, std::string_view shortname,
                   command_line& cmdline, T default_value = T { 0 });
    virtual void parse(__details::command_line_impl&) override final;
    virtual ~multi_argument() override final;
  private:
    T value;
  };

  template <typename T>
  class positional_argument : public __details::argument,
                              public __details::argument_template<T> {
  public:
    positional_argument(command_line& cmdline, T default_value = T { 0 });
    virtual void parse(__details::command_line_impl&) override final;
    virtual ~positional_argument() override final;
  private:
    T value;
  };

  class switch_argument : public __details::argument,
                          public __details::argument_template<bool> {
  public:
    switch_argument(std::string_view longname, std::string_view shortname,
                    command_line& cmdline, bool default_value = false);
    virtual void parse(__details::command_line_impl&) override final;
    virtual ~switch_argument() override;
  private:
    bool value;
  };

} // namespace argp

#endif
