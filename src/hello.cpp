#include <iostream>
#include <vector>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

int main(int argc, char* argv[]) {
  std::vector args(argv, argv + argc);
  json j{{"Hello", "World"}, {"args", args}};
  std::cout << j << "\n";
}
