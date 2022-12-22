#include <span>
#include <string>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace hello {
nlohmann::json greet(std::span<std::string> args);
}
