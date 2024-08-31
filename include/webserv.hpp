#ifndef WEBSERV_HPP_
#define WEBSERV_HPP_

#ifdef __APPLE__
#include <sys/event.h>
#elif __linux__
#include <sys/epoll.h>
#endif

#include <stdexcept>
#include <string>

#include "config.hpp"
#include "server.hpp"
#include "cgi.hpp"

class Webserv {
 private:
  std::vector<Server> servers_;

  void createServerSockets();

#ifdef __APPLE__
  int kq_;
  std::vector<struct kevent> events_;
#elif __linux__
  int epoll_fd_;
  std::vector<struct epoll_event> events_;
#endif
  std::vector<char> buffer_;

  static const int kBufferSize = 1024;
  static const int kMaxEvents = 16;

 public:
  Webserv(const std::string &configFile);

  void handleNewConnection(int server_fd);
  void handleClientData(int client_fd);
  void handleCGIRequest(const HttpRequest& request, int client_fd, const Location *location);

  void run();
};

#endif  // WEBSERV_HPP_
