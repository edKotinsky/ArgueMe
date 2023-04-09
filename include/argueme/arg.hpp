#ifndef ARGUEMEFWD_HPP
#define ARGUEMEFWD_HPP

#include <exception>
#include <functional>
#include <map>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
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

  enum class prefix_policy { require, do_not_require, optional };

  namespace details {

    template <typename T>
    class argument_template {
    public:
      argument_template(T default_value) : value(default_value) {}

      T const& get() const noexcept { return value; }

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

      virtual ~argument() {};
    };

    class named_argument : public argument {
    public:
      named_argument(std::string_view longname, std::string_view shortname,
                     prefix_policy prefix = prefix_policy::optional)
          : lname(longname), sname(shortname), p_policy(prefix) {}

      std::string_view longname() const noexcept { return lname; }

      std::string_view shortname() const noexcept { return sname; }

      std::string_view description() const noexcept { return desc; }

      bool check_prefix(bool has_prefix) const noexcept {
        if ((has_prefix && p_policy == prefix_policy::do_not_require) ||
            (!has_prefix && p_policy == prefix_policy::require))
          return false;
        return true;
      }

      void add_description(std::string_view description) {
        desc = description;
      }
    protected:
      std::string_view lname;
      std::string_view sname;
      std::string_view desc;
      prefix_policy p_policy;
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
       * there is a least one mandatory argument remained, then throws
       * exception `argument_error`.
       */
      inline void parse(svvec_t::const_iterator begin,
                        svvec_t::const_iterator end) {
        if (parsing_active)
          throw command_line_error(
              "command_line_impl::parse called recursively");
        parsing_active = true;

        try {
          current = begin;
          this->end = end;
          cur_pos_arg = p_args.begin();

          while (current != end) {
            auto arg_data = remove_prefix(*current);
            auto arg = arg_data.first;
            bool has_prefix = arg_data.second;
            auto it = args.find(arg);

            if (!arg_at(it).check_prefix(has_prefix))
              throw argument_error("Prefix error", *current);

            std::string_view last_arg = *current;
            try {
              if (it != args.end()) {
                arg_at(it).parse(*this);
              } else if (cur_pos_arg != p_args.end()) {
                cur_pos_arg->get().parse(*this);
                ++cur_pos_arg;
              } else throw argument_error("Unrecognized argument");
            } catch (argument_error const& e) {
              throw argument_error(e.what(), last_arg);
            }
            if (!parsing_active) return;
            ++current;
          }

          while (cur_pos_arg != p_args.end()) {
            if (cur_pos_arg->is_mandatory())
              throw argument_error("Positional argument required");
          }
        } catch (...) {
          parsing_active = false;
          throw;
        }
        parsing_active = false;
      }

      /*
       * Checks if `s` is an argument.
       *
       * Removes a prefix, if it is found in `s`, and finds the resulting
       * string in an args dictionary. If an argument is found, then `s` is an
       * argument name.
       */
      bool is_argument(std::string_view s) {
        auto name = remove_prefix(s).first;
        if (args.find(name) != args.end()) return true;
        return false;
      }

      /*
       * Checks, if `s` starts with longname or shortname prefix. If so,
       * returns `std::string_view` without these first characters. Otherwise,
       * returns `s` itself.
       */
      std::pair<std::string_view, bool> remove_prefix(std::string_view s) {
        bool starts_lname = starts_with(s, lname_prefix);
        bool starts_sname = starts_with(s, sname_prefix);
        if (starts_sname && !starts_lname) {
          return { s.substr(sname_prefix.size()), true };
        } else if (starts_lname) {
          return { s.substr(lname_prefix.size()), true };
        }
        return { s, false };
      }

      /*
       * Checks if `str` starts with `subs`.
       */
      constexpr bool starts_with(std::string_view str, std::string_view subs) {
        return str.substr(0, subs.size()) == subs;
      }

      /*
       * Increments an input vector iterator and returns optional of the next
       * argument. If an end of input vector is reached, then assigns to an
       * input vector's pointer a null value. In the case if input vector is
       * null, returns empty optional.
       */
      std::optional<std::string_view> next_argument() {
        if (parsing_active && ++current != end) { return *current; }
        parsing_active = false;
        return {};
      }

      /*
       * Returns optional of the current argument's string_view. If an end of
       * input vector is reached or input vector is null, then returns an empty
       * optional.
       */
      std::optional<std::string_view> get_argument() {
        if (parsing_active && current != end) { return *current; }
        parsing_active = false;
        return {};
      }

      /*
       * Attaches named argument
       */
      void attach_argument(details::named_argument& arg) {
        args.insert({ arg.longname(), arg });
        args.insert({ arg.shortname(), arg });
        args_list.push_back(arg);
      }

      /*
       * Attaches positional argument. Positional arguments have no names and
       * they are identified only by its position in the vector `p_args`.
       */
      void attach_argument(details::argument& arg, bool arg_mandatory) {
        if (!p_args.empty()) {
          bool prev_arg_necessarity = p_args.front().is_mandatory();
          if (!prev_arg_necessarity && arg_mandatory)
            throw command_line_error("Mandatory positional argument can not "
                                     "follow the not mandatory");
        }
        p_args.emplace_back(arg, arg_mandatory);
      }

      template <class Iterator>
      named_argument& arg_at(Iterator it) const noexcept {
        return it->second.get();
      }

      std::vector<std::string> description() const {
        std::vector<std::string> vec;
        vec.reserve(args_list.size());

        std::string_view delim = ", ";
        const int lefthand_side_length = 30;

        auto calculate_size = [&](named_argument const& arg) {
          return arg.shortname().size() + sname_prefix.size() +
                 arg.longname().size() + lname_prefix.size() + delim.size();
        };

        for (auto const& it : args_list) {
          named_argument const& arg = it.get();
          int argnames_length = calculate_size(arg);
          int description_delimiter = lefthand_side_length - argnames_length;

          if (description_delimiter > 0)
            vec.emplace_back(lefthand_side_length + arg.description().size(),
                             '\0');
          else
            vec.emplace_back(lefthand_side_length + argnames_length +
                                 arg.description().size() + 1,
                             '\0');

          std::string& s = vec.back();

          if (!arg.shortname().empty())
            s.append(sname_prefix).append(arg.shortname()).append(delim);
          if (!arg.longname().empty())
            s.append(lname_prefix).append(arg.longname());

          if (description_delimiter > 0) s.append(description_delimiter, ' ');
          else s.append("\n").append(lefthand_side_length, ' ');

          s.append(arg.description());
        }

        return vec;
      }

      void stop() noexcept { parsing_active = false; }

      std::pair<svvec_t::const_iterator, svvec_t::const_iterator>
          get_arg_iterator() const noexcept {
        return { current, end };
      }

    private:
      struct posarg_wrapper {
      public:
        posarg_wrapper(argument& arg, bool mandatory) noexcept
            : arg(arg), mandatory(mandatory) {}

        argument& get() const noexcept { return arg.get(); }

        bool is_mandatory() const noexcept { return mandatory; }

      private:
        std::reference_wrapper<argument> arg;
        bool const mandatory;
      };

      bool parsing_active = false;

      using argument_t = std::reference_wrapper<named_argument>;
      using argsvec_t = std::vector<argument_t>;

      typename svvec_t::const_iterator current;
      typename svvec_t::const_iterator end;

      using pargsvec_t = std::vector<posarg_wrapper>;
      pargsvec_t p_args;
      typename pargsvec_t::const_iterator cur_pos_arg;

      std::vector<argument_t> args_list;
      std::map<std::string_view, argument_t> args;

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

    template <class C>
    constexpr bool has_operator_extraction_v =
        has_operator_extraction<C>::value;

  } // namespace details

  namespace util {

    template <typename T>
    T from_string(std::string_view s) {
      if constexpr (std::is_same_v<T, std::string> ||
                    std::is_same_v<T, std::string_view>) {
        return std::string { s };
      } else {
        static_assert(details::has_operator_extraction_v<T>,
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

    template <class Functor, typename... Args>
    auto callable_wrapper(Functor f, Args&&... args) {
      return [f, &args...]() {
        return std::invoke(f, args...);
      };
    }

    inline void execute() noexcept {}

    template <class Command, class... Rest>
    inline void execute(Command& cmd, Rest&... cmds) {
      if (cmd.activited()) cmd.execute();
      execute(cmds...);
    }

  } // namespace util

  class command_line {
  public:
    using str_view_vec_t = typename details::command_line_impl::svvec_t;

    command_line(std::string_view longname_prefix,
                 std::string_view shortname_prefix)
        : impl(longname_prefix, shortname_prefix), longname_p(longname_prefix),
          shortname_p(shortname_prefix) {}

    /*
     * Attaches a named argument. Intended for internal usage, shall be called
     * only by arguments.
     */
    void attach(details::named_argument& arg) { impl.attach_argument(arg); }

    /*
     * Attaches a positional argument. Intended for internal usage, shall be
     * called only by arguments.
     */
    void attach(details::argument& arg, bool mandatory) {
      impl.attach_argument(arg, mandatory);
    }

    void parse(std::vector<std::string_view> const& vec) {
      impl.parse(vec.cbegin(), vec.cend());
    }

    void parse(char** argv, int argc) {
      std::vector<std::string_view> args;
      args.reserve(argc - 1);
      for (int i = 1; i < argc; ++i) args.push_back(argv[i]);
      impl.parse(args.cbegin(), args.cend());
    }

    void parse(const std::vector<std::string>& vec) {
      std::vector<std::string_view> args;
      args.reserve(vec.size());
      for (std::string_view s : vec) args.push_back(s);
      impl.parse(args.cbegin(), args.cend());
    }

    void parse(str_view_vec_t::const_iterator begin,
               str_view_vec_t::const_iterator end) {
      impl.parse(begin, end);
    }

    void stop() noexcept { impl.stop(); }

    std::pair<str_view_vec_t::const_iterator, str_view_vec_t::const_iterator>
        get_iterator() const noexcept {
      return impl.get_arg_iterator();
    }

    std::string_view prefix_long() const noexcept { return longname_p; }

    std::string_view prefix_short() const noexcept { return shortname_p; }

    std::vector<std::string> description() const { return impl.description(); }

  private:
    details::command_line_impl impl;
    std::string_view longname_p;
    std::string_view shortname_p;
  };

  template <typename T>
  class value_argument : public details::named_argument,
                         public details::argument_template<T> {
  public:
    value_argument(std::string_view longname, std::string_view shortname,
                   command_line& cmdline,
                   prefix_policy prefix = prefix_policy::optional,
                   T default_value = T {})
        : details::named_argument(longname, shortname, prefix),
          details::argument_template<T>(default_value) {
      cmdline.attach(*this);
    }

    virtual void parse(details::command_line_impl& cmdline) override final {
      if (activited) throw argument_error("Option can be appeared only once");
      activited = true;
      auto s = cmdline.next_argument();
      if (!s || cmdline.is_argument(*s))
        throw argument_error("Option requires a value");
      this->value = util::from_string<T>(*s);
    }

    virtual ~value_argument() override {}
  private:
    bool activited = false;
  };

  template <typename T>
  class multi_argument : public details::named_argument {
  public:
    multi_argument(std::string_view longname, std::string_view shortname,
                   command_line& cmdline,
                   prefix_policy prefix = prefix_policy::optional)
        : details::named_argument(longname, shortname, prefix) {
      cmdline.attach(*this);
    }

    virtual void parse(details::command_line_impl& cmdline) override final {
      auto s = cmdline.next_argument();
      if (!s || cmdline.is_argument(*s))
        throw argument_error("Option requires a value");
      T value = util::from_string<T>(*s);
      this->value.push_back(value);
    }

    virtual ~multi_argument() override {};

    std::vector<T> const& get() const noexcept { return value; }
  private:
    std::vector<T> value;
  };

  template <typename T>
  class positional_argument : public details::argument,
                              public details::argument_template<T> {
  public:
    positional_argument(command_line& cmdline, bool is_mandatory = false,
                        T default_value = T {})
        : details::argument_template<T>(default_value) {
      cmdline.attach(*this, is_mandatory);
    }

    virtual void parse(details::command_line_impl& cmdline) override final {
      auto s = cmdline.get_argument();
      if (!s) throw argument_error("Option requires a value");
      this->value = util::from_string<T>(*s);
    }

    virtual ~positional_argument() override {}
  };

  class switch_argument : public details::named_argument,
                          public details::argument_template<bool> {
  public:
    switch_argument(std::string_view longname, std::string_view shortname,
                    command_line& cmdline,
                    prefix_policy prefix = prefix_policy::optional,
                    bool default_value = false)
        : details::named_argument(longname, shortname, prefix),
          details::argument_template<bool>(default_value) {
      value = default_value;
      cmdline.attach(*this);
    }

    virtual void parse(details::command_line_impl&) override final {
      value = !value;
    }

    virtual ~switch_argument() override {}
  };

  class deferred_execution {
  protected:
    template <class Functor>
    void execute_now(Functor) noexcept {}

    template <class Functor>
    void execute_deffered(Functor functor) {
      std::invoke(functor);
    }
  };

  class instant_execution {
  protected:
    template <class Functor>
    void execute_now(Functor functor) {
      std::invoke(functor);
    }
  };

  template <class Functor, class ExecutionPolicy = instant_execution>
  class command : public details::named_argument,
                  ExecutionPolicy {
  public:
    command(std::string_view longname, std::string_view shortname,
            command_line& cmdline, Functor functor,
            ExecutionPolicy = instant_execution {},
            prefix_policy prefix = prefix_policy::optional)
        : details::named_argument(longname, shortname, prefix), f(functor) {
      cmdline.attach(*this);
    }

    virtual void parse(details::command_line_impl&) override final {
      this->execute_now(f);
      active = true;
    }

    void execute() { this->execute_deffered(f); }

    bool activited() const noexcept { return active; }

    virtual ~command() override {}
  private:
    Functor f;
    bool active = false;
  };

} // namespace arg

#endif
