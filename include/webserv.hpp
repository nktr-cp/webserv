#ifndef WEBSERV_HPP_
#define WEBSERV_HPP_

#include <sys/event.h>

#include <string>

#include "server.hpp"

class Webserv {
 private:
  std::vector<Server> servers_;
  void parseConfig(const std::string &configFile);

  void createServerSockets();

  int kq_;
  std::vector<char> buffer_;
  std::vector<struct kevent> events_;

  static const int kBufferSize = 1024;
  static const int kMaxEvents = 16;

 public:
  Webserv(const std::string &configFile);

  void handleNewConnection(int server_fd);
  void handleClientData(int client_fd);

  void run();
};

#endif  // WEBSERV_HPP_
