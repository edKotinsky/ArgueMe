#include <argueme/arg.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

TEST_CASE("value_argument") {
  arg::command_line cmd("--", "-");

  SECTION("If argument is not found and there is no positional args") {
    std::vector<std::string_view> vec { "--hello" };

    REQUIRE_THROWS_AS(cmd.parse(vec), arg::argument_error);
  }

  SECTION("Argument with long name") {
    std::vector<std::string_view> vec { "--hello", "world" };

    arg::value_argument<std::string> hello_world("hello", "h", cmd);

    REQUIRE_NOTHROW(cmd.parse(vec));
    REQUIRE(hello_world.get() == "world");
  }

  SECTION("Argument with short name") {
    std::vector<std::string_view> vec { "-h", "world" };

    arg::value_argument<std::string> hello_world("hello", "h", cmd);

    REQUIRE_NOTHROW(cmd.parse(vec));
    REQUIRE(hello_world.get() == "world");
  }

  SECTION("value_argument can be appeared only once") {
    std::vector<std::string_view> vec { "--hello", "world", "--hello",
                                        "error" };

    arg::value_argument<std::string> hello_world("hello", "h", cmd);

    REQUIRE_THROWS(cmd.parse(vec));
  }

  SECTION("value_argument requires a value after itself") {
    std::vector<std::string_view> vec { "--hello" };

    arg::value_argument<std::string> hello_world("hello", "h", cmd);

    REQUIRE_THROWS(cmd.parse(vec));
  }

  SECTION("value_argument requires a value") {
    std::vector<std::string_view> vec { "-i", "-f" };

    arg::value_argument<int> i("int", "i", cmd);
    arg::value_argument<float> f("float", "f", cmd);

    REQUIRE_THROWS(cmd.parse(vec));
  }

  SECTION("Incorrect long name prefix 1") {
    std::vector<std::string_view> vec { "-hello" };

    arg::value_argument<std::string> hello_world("hello", "h", cmd);

    REQUIRE_THROWS(cmd.parse(vec));
  }

  SECTION("Incorrect long name prefix 2") {
    std::vector<std::string_view> vec { "---hello" };

    arg::value_argument<std::string> hello_world("hello", "h", cmd);

    REQUIRE_THROWS(cmd.parse(vec));
  }

  SECTION("Incorrect short name prefix") {
    std::vector<std::string_view> vec { "--h" };

    arg::value_argument<std::string> hello_world("hello", "h", cmd);

    REQUIRE_THROWS(cmd.parse(vec));
  }

  SECTION("Different values") {
    std::vector<std::string_view> vec { "--int", "123", "--float", "3.14" };

    arg::value_argument<int> i("int", "i", cmd);
    arg::value_argument<float> f("float", "f", cmd);

    cmd.parse(vec);

    REQUIRE(i.get() == 123);
    REQUIRE_THAT(f.get(), Catch::Matchers::WithinRel(3.14f));
  }

  SECTION("Incorrect value") {
    std::vector<std::string_view> vec { "--int", "3.14" };

    arg::value_argument<int> i("int", "i", cmd);

    REQUIRE_THROWS(cmd.parse(vec));
  }
}
