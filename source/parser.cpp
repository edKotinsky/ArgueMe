#include <argp/parser.hpp>

namespace argp {

  namespace __details {

    void __details::command_line_impl::parse(const svvec_t& input_vec) {
      input = &input_vec;
      current = input->begin();

      while (current != input->end()) {
        auto arg = remove_prefix(*current);
        auto it = args.find(arg);
        if (it != args.end()) {
          it->second.get().parse(*this);
        } else if (cur_pos_arg != p_args.end()) {
          cur_pos_arg->get().parse(*this);
          ++cur_pos_arg;
        } else {
          throw command_line_error("Unexpected argument", *current);
        }
        ++current;
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
      } else if (starts_lname && !starts_sname) {
        return s.substr(lname_prefix.size());
      }
      return s;
    }

    void command_line_impl::attach_argument(std::string_view lname,
                                            std::string_view sname,
                                            __details::argument& arg) {
      args.insert({ lname, arg });
      args.insert({ sname, arg });
    }

    void command_line_impl::attach_pos_argument(__details::argument& arg) {
      p_args.push_back(arg);
    }

  } // namespace __details

} // namespace argp
