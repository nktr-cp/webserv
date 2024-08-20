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

class Server {
 private:
  std::vector<ServerConfig> config_;
  int server_fd_;
  struct sockaddr_in address_;

 public:
  Server(std::vector<ServerConfig> config);
  ~Server();

  void createSocket();

  int getServerFd() const;
};

#endif  // SERVER_HPP_
