#include "hello.h"

namespace hello {
nlohmann::json greet(std::span<std::string> args) {
  return json{{"Hello", "World"}, {"args", args}};
}
}
