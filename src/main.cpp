#include "webserv.hpp"

int main(int ac, char **av) {
  if (ac > 2) {
    std::cerr << "Usage: " << av[0] << " [config_file]" << std::endl;
    return 1;
  }

  std::string configFile;
  if (ac == 1) {
    configFile = "config/default.conf";
  } else {
    configFile = av[1];
  }

  Webserv webserv;
  try {
    webserv = Webserv(configFile);
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
  webserv.run();
  return 0;
}