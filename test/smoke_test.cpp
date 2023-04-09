#include <argueme/arg.hpp>
#include <cassert>

int main() {
  int i = arg::util::from_string<int>("123");
  assert(i == 123);

  arg::command_line cmd("--", "-");
  arg::switch_argument sw("hello", "h", cmd);

  std::vector<std::string_view> vec = { "--hello" };

  assert(sw.get() == false);

  cmd.parse(vec);

  assert(sw.get() == true);

  std::vector<std::string_view> vec1 = { "-h" };

  cmd.parse(vec);

  assert(sw.get() == false);

  return 0;
}
