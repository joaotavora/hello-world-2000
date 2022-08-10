#include "hello.h"

json greet(std::span<std::string> args) {
  return json{{"Hello", "World"}, {"args", args}};
}
