#include <argueme/arg.hpp>
#include <catch2/catch_test_macros.hpp>

using svvec_t = std::vector<std::string_view>;

void function(bool& called) { called = true; }

struct TestStruct {
  void method(bool& called) { called = true; }
};

TEST_CASE("Commands") {
  arg::command_line cmd("--", "-");

  TestStruct ts;

  SECTION("Function call") {
    bool called = false;
    arg::command fp("function", "f", cmd,
                    arg::util::callable_wrapper(&function, called));
    svvec_t vec { "--function" };
    cmd.parse(vec);
    REQUIRE(called == true);
  }

  SECTION("Method call") {
    bool called = false;
    arg::command mfp(
        "method", "m", cmd,
        arg::util::callable_wrapper(&TestStruct::method, ts, called));
    svvec_t vec { "--method" };
    cmd.parse(vec);
    REQUIRE(called == true);
  }
}
