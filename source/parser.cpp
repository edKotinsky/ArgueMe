#include <argp/parser.hpp>

void argp::__details::command_line_impl::parse(const svvec_t& input_vec) {
  input = &input_vec;
  current = input->begin();

  while (current != input->end()) {
    auto it = args.find(*current);
    if (it != args.end()) it->second.get().parse(*this);
    else if (cur_pos_arg != p_args.end()) {
      cur_pos_arg->get().parse(*this);
    } else
      throw;
  }
}
