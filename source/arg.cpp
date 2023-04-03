#include "argueme/arg.hpp"
#include <cstdio>

namespace arg {

  namespace utility {

    void utility::command_line_impl::parse(const svvec_t& input_vec) {
      input = &input_vec;
      current = input->begin();
      cur_pos_arg = p_args.begin();

      while (current != input->end()) {
        auto arg = remove_prefix(*current);
        auto it = args.find(arg);
        try {
          if (it != args.end()) {
            it->second.get().parse(*this);
          } else if (cur_pos_arg != p_args.end()) {
            cur_pos_arg->get().parse(*this);
            ++cur_pos_arg;
          } else throw argument_error("Unrecognized argument");
        } catch (argument_error const& e) {
          throw argument_error(e.what(), *current);
        }
        ++current;
      }

      while (cur_pos_arg != p_args.end()) {
        if (cur_pos_arg->is_mandatory())
          throw argument_error("Positional argument required");
      }
    }

    bool command_line_impl::is_argument(std::string_view s) {
      auto name = remove_prefix(s);
      if (args.find(name) != args.end()) return true;
      return false;
    }

    std::optional<std::string_view> command_line_impl::next_argument() {
      if (input && current != input->end()) {
        ++current;
        return *current;
      }
      return {};
    }

    std::optional<std::string_view> command_line_impl::get_argument() {
      if (input && current != input->end()) return *current;
      return {};
    }

    constexpr bool command_line_impl::starts_with(std::string_view str,
                                                  std::string_view subs) {
      return str.substr(0, subs.size()) == subs;
    }

    std::string_view command_line_impl::remove_prefix(std::string_view s) {
      bool starts_lname = starts_with(s, lname_prefix);
      bool starts_sname = starts_with(s, sname_prefix);
      if (starts_sname && !starts_lname) {
        return s.substr(sname_prefix.size());
      } else if (starts_lname) {
        return s.substr(lname_prefix.size());
      }
      return s;
    }

    void command_line_impl::attach_argument(std::string_view lname,
                                            std::string_view sname,
                                            utility::argument& arg) {
      args.insert({ lname, arg });
      args.insert({ sname, arg });
    }

    void command_line_impl::attach_argument(utility::argument& arg,
                                            bool arg_mandatory) {
      if (!p_args.empty()) {
        bool prev_arg_necessarity = p_args.front().is_mandatory();
        if (!prev_arg_necessarity && arg_mandatory)
          throw command_line_error(
              "Mandatory positional argument can not follow the not mandatory");
      }
      p_args.emplace_back(arg, arg_mandatory);
    }

  } // namespace utility

  void command_line::attach(std::string_view longname,
                            std::string_view shortname,
                            utility::argument& arg) {
    impl.attach_argument(longname, shortname, arg);
  }

  void command_line::attach(utility::argument& arg, bool mandatory) {
    impl.attach_argument(arg, mandatory);
  }

  void command_line::parse(std::vector<std::string_view> const& vec) {
    impl.parse(vec);
  }

  void command_line::parse(const char** argv, int argc) {
    std::vector<std::string_view> args;
    args.reserve(argc - 1);
    for (int i = 1; i < argc; ++i) args.push_back(argv[i]);
    impl.parse(args);
  }

  void command_line::parse(const std::vector<std::string>& vec) {
    std::vector<std::string_view> args;
    args.reserve(vec.size());
    for (std::string_view s : vec) args.push_back(s);
    impl.parse(args);
  }

  void switch_argument::parse(utility::command_line_impl&) { value = !value; }

  switch_argument::~switch_argument() {}

} // namespace arg
