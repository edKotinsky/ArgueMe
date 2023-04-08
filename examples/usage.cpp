/*
 * ## Usage example of ArgueMe library.
 *
 * There are several cases with different behavior of a program:
 *
 * 1) Program name itself without any arguments: ./UsageExample
 *    will cause help print:
 *
 * ```
 * $ ./UsageExample
 * /help message/
 * ```
 *
 * 2) Invalid arguments (nonexistent arguments) will cause short error message
 *    and help print:
 *
 * ```
 * $ ./UsageExample -g
 * Invalid argument: -g
 * /help message/
 * ```
 *
 * 3) -h and --help arguments will cause help message print.
 *
 * ```
 * $ ./UsageExample -h
 * /help message/
 * ```
 *
 * 4) Arguments -i|--int, -f|--float without a following value (`./UsageExample
 *    -i`) cause an error message and help message print.
 *
 * ```
 * $ ./UsageExample -i
 * Option requires a value: -i
 * /help message/
 * ```
 *
 * 5) Argument -i|--int with the proper value: -i|--int <value> and the same
 *    with -f|--float argument cause the following output:
 *
 * ```
 * $ ./UsageExample -i 10
 * Hello int: 10
 * $ ./UsageExample -f 3.14
 * Hello float: 3.14
 * ```
 *
 * 6) If a given value string cannot be converted to a type of an argument,
 *    program will give the following output:
 *
 * ```
 * $ ./UsageExample -i 3.14
 * Cannot convert a string `3.14` to a value: -i
 * /help message/
 * ```
 *
 * 7) If the same argument appeared more than once:
 *
 * ```
 * $ ./UsageExample -i 1 -i 2
 * Option can be appeared only once: -i
 * /help message/
 * ```
 *
 * 8) User can pass both -i|--int and -f|--float arguments, but it does not work
 * with -h|--help argument:
 *
 * ```
 * $ ./UsageExample -i 1 -f 1.41
 * Hello int: 1
 * Hello float: 1.41
 * $ ./UsageExample -f 1.41 -h
 * /help message/
 * ```
 *
 * In all code examples above the `/help message/` string stands for:
 *
 * ```
 * -i, --int   Prints "Hello int: <value>"
 * -f, --float Prints "Hello float: <value>"
 * -h, --help  Prints help message
 * ```
 */
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
