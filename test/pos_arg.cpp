#include <argueme/arg.hpp>
#include <catch2/catch_test_macros.hpp>
#include <string>

TEST_CASE("positional_argument") {
  arg::command_line cmd("--", "-");

  using svvec_t = std::vector<std::string_view>;

  SECTION("pos args have no name and they're identified by its position") {
    svvec_t vec { "hello", "world" };

    arg::positional_argument<std::string> hello(cmd);
    arg::positional_argument<std::string> world(cmd);

    cmd.parse(vec);

    CHECK(hello.get() == "hello");
    CHECK(world.get() == "world");
  }
}
