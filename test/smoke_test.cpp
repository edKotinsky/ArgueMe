#include <argp/command_line.hpp>
#include <cassert>

int main() {
  int i = argp::utility::from_string<int>("123");
  assert(i == 123);
  return 0;
}
