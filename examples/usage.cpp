#include <argueme/arg.hpp>
#include <iostream>

void print_help(arg::command_line const& cmd) {
  auto descs = cmd.description();
  for (std::string const& s : descs) std::cout << s << std::endl;
}

int main(int argc, char** argv) {
  arg::command_line cmd("--", "-");

  arg::value_argument<int> hello_int("int", "i", cmd);
  hello_int.add_description("Prints \"Hello int: <value>\"");

  arg::value_argument<float> hello_float("float", "f", cmd);
  hello_float.add_description("Prints \"Hello float: <value>\"");

  arg::switch_argument help("help", "h", cmd);
  help.add_description("Prints help message");

  try {
    cmd.parse(argv, argc);
  } catch (arg::argument_error const& e) {
    std::cout << e.what() << ": " << e.argname() << std::endl;
    print_help(cmd);
  }

  if (help.get() || argc == 1) {
    print_help(cmd);
    return 0;
  }

  if (hello_int.get())
    std::cout << "Hello int: " << hello_int.get() << std::endl;

  if (hello_float.get())
    std::cout << "Hello float: " << hello_float.get() << std::endl;

  return 0;
}
