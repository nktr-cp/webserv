#include "server.hpp"

Server::Server(ServerConfig config) : config_(config), type_(kServer) {
  struct addrinfo *address;
  struct addrinfo hints = {
      .ai_family = AF_INET,
      .ai_socktype = SOCK_STREAM,
      .ai_flags = AI_PASSIVE,
  };

  std::string host = config.getHost();
  std::string port = config.getPort();
  if (getaddrinfo(host.c_str(), port.c_str(), &hints, &address) != 0) {
    throw SysCallFailed();
  }

  sock_fd_ =
      socket(address->ai_family, address->ai_socktype, address->ai_protocol);
  if (sock_fd_ == -1) {
    freeaddrinfo(address);
    throw SysCallFailed();
  }

  fcntl(sock_fd_, F_SETFL, O_NONBLOCK);

  if (bind(sock_fd_, address->ai_addr, address->ai_addrlen) == -1) {
    freeaddrinfo(address);
    throw SysCallFailed();
  }

  int opt = 1;
  setsockopt(sock_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  // set backlog to 10
  if (listen(sock_fd_, 10) == -1) {
    throw SysCallFailed();
  }

  // debug message
  std::cout << "Server started at " << host << ":" << port << std::endl;
}

int Server::getSockfd() const { return sock_fd_; }

SocketType Server::getType() const { return type_; }
