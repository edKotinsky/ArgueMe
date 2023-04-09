#include <argueme/arg.hpp>
#include <catch2/catch_test_macros.hpp>

using svvec_t = std::vector<std::string_view>;

TEST_CASE("Prefix required") {
  arg::command_line cmd("--", "-");

  arg::value_argument<int> ia("int", "i", cmd, arg::prefix_policy::require);
  arg::value_argument<std::string_view> sva("string", "s", cmd,
                                            arg::prefix_policy::require);

  SECTION("Correct prefix 1") {
    svvec_t vec { "-i", "1", "-s", "Hello world" };
    cmd.parse(vec);
    CHECK(ia.get() == 1);
    CHECK(sva.get() == "Hello world");
  }

  SECTION("Correct prefix 2") {
    svvec_t vec { "--int", "1", "--string", "Hello world" };
    cmd.parse(vec);
    CHECK(ia.get() == 1);
    CHECK(sva.get() == "Hello world");
  }

  SECTION("Incorrect prefix 1") {
    svvec_t vec { "int", "1", "string", "Hello world" };
    REQUIRE_THROWS_AS(cmd.parse(vec), arg::argument_error);
  }

  SECTION("Incorrect prefix 2") {
    svvec_t vec { "i", "1", "s", "Hello world" };
    REQUIRE_THROWS_AS(cmd.parse(vec), arg::argument_error);
  }
}

TEST_CASE("Prefix does not required") {
  arg::command_line cmd("--", "-");

  arg::value_argument<int> ia("int", "i", cmd,
                              arg::prefix_policy::do_not_require);
  arg::value_argument<std::string_view> sva(
      "string", "s", cmd, arg::prefix_policy::do_not_require);

  SECTION("Correct prefix 1") {
    svvec_t vec { "i", "1", "s", "Hello world" };
    cmd.parse(vec);
    CHECK(ia.get() == 1);
    CHECK(sva.get() == "Hello world");
  }

  SECTION("Correct prefix 2") {
    svvec_t vec { "int", "1", "string", "Hello world" };
    cmd.parse(vec);
    CHECK(ia.get() == 1);
    CHECK(sva.get() == "Hello world");
  }

  SECTION("Incorrect prefix 1") {
    svvec_t vec { "--int", "1", "--string", "Hello world" };
    REQUIRE_THROWS_AS(cmd.parse(vec), arg::argument_error);
  }

  SECTION("Incorrect prefix 2") {
    svvec_t vec { "-i", "1", "-s", "Hello world" };
    REQUIRE_THROWS_AS(cmd.parse(vec), arg::argument_error);
  }
}
