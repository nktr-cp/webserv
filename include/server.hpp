#ifndef SERVER_HPP_
#define SERVER_HPP_

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <stdexcept>

class Server {
 private:
  int port_;
  int server_fd_;
  struct sockaddr_in address_;

 public:
  Server();
  ~Server();

  void createSocket();

  int getServerFd() const;
};

#endif  // SERVER_HPP_
