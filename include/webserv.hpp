#ifndef WEBSERV_HPP_
#define WEBSERV_HPP_

#include <string>

class Webserv {
 private:
  // std::vector<Server> servers_;
  void parse_config(const std::string &config_file);

 public:
  Webserv(const std::string &config_file);
  void run();
};

#endif  // WEBSERV_HPP_
