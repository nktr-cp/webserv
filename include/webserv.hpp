#ifndef WEBSERV_HPP_
#define WEBSERV_HPP_

#ifdef __APPLE__
#include <sys/event.h>
#elif __linux__
#include <sys/epoll.h>
#endif

#include <sys/time.h>

#include <stdexcept>
#include <string>
#include <map>

#include "config.hpp"
#include "server.hpp"
#include "http_request.hpp"
#include "http_response.hpp"

class Webserv {
 private:
  std::vector<Server> servers_;
  std::map<int, time_t> connections_;

  void createServerSockets();
  void sendResponse(const int client_fd, const HttpResponse &response, bool keepAlive);
  void handleTimeout();
  void closeConnection(int sock_fd);

#ifdef __APPLE__
  int kq_;
  std::vector<struct kevent> events_;
#elif __linux__
  int epoll_fd_;
  std::vector<struct epoll_event> events_;
#endif
  std::vector<char> buffer_;

  static const int kBufferSize = 1;
  static const int kMaxEvents = 16;
  static const int kTimeoutSec = 120;
  static const int kWaitTime = 1000;

 public:
  Webserv();
  Webserv(const std::string &configFile);

  void handleNewConnection(int server_fd);
  void handleClientData(int client_fd);

  void run();
};

#endif  // WEBSERV_HPP_
