#include <vector>
#include <string>

#define CATCH_CONFIG_RUNNER
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_session.hpp>

#include "core/hello.h"

TEST_CASE("Greeting is acceptable", "[core]") {
  auto args = std::vector<std::string>{"foo", "bar"};
  auto g = hello::greet(args);
  REQUIRE(g.contains("Hello"));
  REQUIRE(g.at("Hello") == "World");
  REQUIRE(g.contains("args"));
  REQUIRE(g.at("args").is_array());
  REQUIRE(g.at("args").at(0) == "foo");
  REQUIRE(g.at("args").at(1) == "bar");
}

int main(int argc, char** argv)
{
  int result = Catch::Session().run( argc, argv );
  if (result != 0) return result;
}
