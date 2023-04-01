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
        lname_start = longname_start;
        sname_start = shortname_start;
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
       * Removes a prefix, if it is found is `s`, and finds the resulting string
       * in an args dictionary. If an argument is found, then `s` is an argument
       * name.
       */
      bool is_argument(std::string_view s);

      /*
       * Increments an input vector iterator and returns optional of the next
       * argument. If an end of input vector is reached, then assigns to an
       * input vector's pointer a null value. In the case if input vector is
       * null, returns empty optional.
       */
      std::optional<std::string_view> next_argument();

      /*
       * Returns the current argument.
       */
      std::string_view get_argument();

    private:
      using argsvec_t = std::vector<argument>;

      svvec_t const* input;
      typename svvec_t::const_iterator current;

      argsvec_t positional_args;
      typename argsvec_t::size_type cur_pos_arg;
      std::unordered_map<std::string_view, std::reference_wrapper<argument>>
          args;

      std::string_view lname_start;
      std::string_view sname_start;
    };

  } // namespace __details

  class command_line {
  public:
    command_line(std::string_view longname_start,
                 std::string_view shortname_start);
    void attach(__details::argument& arg);
    void parse(char const* argv, int argc);
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
