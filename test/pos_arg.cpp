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

  SECTION("Required positional arg") {
    svvec_t vec { "hello" };

    arg::positional_argument<std::string> hello(cmd, true);
    arg::positional_argument<std::string> world(cmd, true);

    REQUIRE_THROWS_AS(cmd.parse(vec), arg::argument_error);
  }

  SECTION("Required pos args can not follow the not required") {
    svvec_t vec { "hello", "world" };

    arg::positional_argument<std::string> not_mandatory(cmd, false);
    REQUIRE_THROWS_AS(arg::positional_argument<std::string>(cmd, true),
                      arg::command_line_error);
  }
}
