#ifndef SERVER_HPP_
#define SERVER_HPP_

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <stdexcept>

#include "config.hpp"
#include "http_request.hpp"
#include "http_response.hpp"
#include "request_handler.hpp"
#include "session.hpp"

class Server {
 private:
  std::vector<ServerConfig> config_;
  SessionManager sessionManager_;
  int server_fd_;
  struct sockaddr_in address_;

 public:
  Server(std::vector<ServerConfig> config);
  ~Server();

  void createSocket();
  RequestHandler getHander(HttpRequest& request, HttpResponse& response);
  void handleRequest(HttpRequest &request, HttpResponse &response, RequestHandler &handler);

  int getServerFd() const;
  std::vector<ServerConfig> getConfig() const;
};

#endif  // SERVER_HPP_
