#include <argueme/arg.hpp>
#include <catch2/catch_test_macros.hpp>

using svvec_t = std::vector<std::string_view>;

TEST_CASE("Stopping parsing") {
  arg::command_line cmd("--", "-");

  auto foo_lambda = [](arg::command_line& cmdline) {
    cmdline.stop();
  };

  auto bar_lambda = []() {
    FAIL("bar_lambda must not be called");
  };

  arg::command foo("foo", "f", cmd,
                   arg::util::callable_wrapper(foo_lambda, cmd));
  arg::command bar("bar", "b", cmd,
                   arg::util::callable_wrapper(bar_lambda));

  svvec_t vec { "--foo", "--bar" };

  cmd.parse(vec);

  SUCCEED("foo command stops parsing process");
}
