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
  // key: client_fd, value: HttpRequest, keepAlive
  std::map<int, std::pair<HttpResponse, bool> > response_buffers_;

  std::map<int, pid_t> fd_v_pid_;
  std::map<int, int> fd_v_client_;

  void createServerSockets();
  void sendResponse(const int client_fd, const HttpResponse &response, bool keepAlive);
  void handleTimeout();
  void closeConnection(int sock_fd);

  void registerSendEvent(int client_fd, const HttpResponse &response, bool keepAlive);

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
  static const int kTimeoutSec = 10;
  static const int kWaitTime = 1000;

 public:
  Webserv();
  Webserv(const std::string &configFile);

  void handleNewConnection(int server_fd);
  void handleClientData(int client_fd);

  void run();
};

#endif  // WEBSERV_HPP_
