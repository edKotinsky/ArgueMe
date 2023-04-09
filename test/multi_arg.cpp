#include <argueme/arg.hpp>
#include <catch2/catch_test_macros.hpp>
#include <string>
#include <string_view>

TEST_CASE("multi_argument") {
  arg::command_line cmd("--", "-");

  using svvec_t = std::vector<std::string_view>;
  using svec_t = std::vector<std::string>;

  SECTION("multi_argument allows to be appeared multiple times") {
    svvec_t vec { "--message", "hello",     "-m",
                                        "beautiful", "--message", "world" };

    arg::multi_argument<std::string> msg("message", "m", cmd);

    cmd.parse(vec);

    svec_t expected {"hello", "beautiful", "world"};
    REQUIRE(msg.get() == expected);
  }

  SECTION("Invalid type") {
    svvec_t vec { "--value", "123", "--value", "1.23" };

    arg::multi_argument<int> v("value", "v", cmd);

    REQUIRE_THROWS(cmd.parse(vec));
  }

  SECTION("multi_argument requires a value 1") {
    svvec_t vec { "--message" };

    arg::multi_argument<std::string_view> msg("message", "m", cmd);

    REQUIRE_THROWS(cmd.parse(vec));
  }

  SECTION("multi_argument requires a value 2") {
    svvec_t vec { "--value", "--value", "123" };

    arg::multi_argument<int> v("value", "v", cmd);

    REQUIRE_THROWS(cmd.parse(vec));
  }
}
