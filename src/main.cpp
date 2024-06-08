#include <iostream>

#include "config.hpp"
#include "server.hpp"

int main(int ac, char** av) {
  if (ac > 2) {
    std::cerr << "Usage: " << av[0] << " <config_file>" << std::endl;
    return 1;
  }

  std::string config_file = ac == 2 ? av[1] : "default.conf";

  try {
    Config config(config_file);
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
}
