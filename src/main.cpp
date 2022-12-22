#include <iostream>
#include <vector>
#include <string>

#include "core/hello.h"

int main(int argc, char* argv[]) {
  std::vector<std::string> args(argv, argv + argc);
  auto j = hello::greet(args);
  std::cout << j << "\n";
}
