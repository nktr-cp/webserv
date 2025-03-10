#include "webserv.hpp"
#include <cstdlib>

const std::string VersionInfo::kProgramName = "webserv";
const std::string VersionInfo::kProgramVersion = "1.0";
const std::string VersionInfo::kHttpVersion = "HTTP/1.1";
const std::string VersionInfo::kCgiVersion = "CGI/1.1";
const std::string VersionInfo::kUrlPrefixSecure = "https://";
const std::string VersionInfo::kUrlPrefix = "http://";

bool checkInterpreter(const std::string &interpreter) {
  std::string command = "which " + interpreter + " > /dev/null 2>&1";
  return system(command.c_str()) == 0;
}

int main(int ac, char **av) {
  if (!checkInterpreter("python") || !checkInterpreter("sh")) {
    std::cerr << VersionInfo::kProgramName << ": " << PYTHON_INTERPRETER << " and " << SHELL_INTERPRETER << " are required to run this program" << std::endl;
    return 1;
  }

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

  try {
    Webserv webserv(configFile);
    webserv.run();
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}