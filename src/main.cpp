#include "webserv.hpp"

const std::string VersionInfo::kProgramName = "webserv";
const std::string VersionInfo::kProgramVersion = "1.0";
const std::string VersionInfo::kHttpVersion = "HTTP/1.1";
const std::string VersionInfo::kCgiVersion = "CGI/1.1";
const std::string VersionInfo::kUrlPrefixSecure = "https://";
const std::string VersionInfo::kUrlPrefix = "http://";

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
    return 1;
  }
  try {
    webserv.run();
  } catch (const std::exception &e) {
    return 1;
  }
  return 0;
}