#include <iostream>

#include "webserv.hpp"

int main(int ac, char **av) {
  if (ac > 2) {
    std::cerr << "Usage: " << av[0] << " [config_file]" << std::endl;
    return 1;
  }

  std::string config_file;
  if (ac == 1) {
    config_file = "config/default.conf";
  } else {
    config_file = av[1];
  }

  try {
    Webserv webserv(config_file);
    webserv.run();
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
}