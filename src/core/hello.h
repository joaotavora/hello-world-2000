#include <span>
#include <string>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

json greet(std::span<std::string> args);
