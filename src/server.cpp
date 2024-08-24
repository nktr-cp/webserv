#include "server.hpp"

void Server::createSocket() {
  // Create a socket file descriptor
  // AF_INET: IPv4, SOCK_STREAM: TCP
  if ((server_fd_ = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    throw std::runtime_error("Socket creation failed");
  }

  // Set socket options
  // SOL_SOCKET: Socket level option
  // SO_REUSEADDR: Allow reuse of local addresse
  int opt = 1;
  if (setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
    close(server_fd_);
    throw std::runtime_error("Setsockopt failed");
  }

  // Configure server address structure
  unsigned int range[] = {0, 65535};
  int port = ft::stoui(config_[0].getPort(), range);
  address_.sin_family = AF_INET;          // IPv4
  address_.sin_addr.s_addr = INADDR_ANY;  // Bind to all available interfaces
  address_.sin_port = htons(port);        // Convert port to network byte order

  // Bind the socket to the specified IP and port
  if (bind(server_fd_, reinterpret_cast<struct sockaddr*>(&address_),
           sizeof(address_)) < 0) {
    close(server_fd_);
    throw std::runtime_error("Bind failed");
  }

  // Start listening for client connections
  // second argument is the maximum length of the queue of pending connections
  if (listen(server_fd_, SOMAXCONN) < 0) {
    close(server_fd_);
    throw std::runtime_error("Listen failed");
  }

  // Set the socket to non-blocking mode
  int flags = fcntl(server_fd_, F_GETFL, 0);
  fcntl(server_fd_, F_SETFL, flags | O_NONBLOCK);
}

Server::Server(std::vector<ServerConfig> config) : config_(config) {}

Server::~Server() {}

int Server::getServerFd() const { return server_fd_; }

std::vector<ServerConfig> Server::getConfig() const { return config_; }

void Server::handleRequest(HttpRequest& request, HttpResponse& response) {
  // 該当するコンフィグを探す
  size_t tgt_index = 0;
  while (tgt_index < config_.size()) {
    if (config_[tgt_index].getServerName() == request.getHostName()) {
      break;
    }
    tgt_index++;
  }
  if (tgt_index == config_.size()) {
    tgt_index = 0;
  }
  RequestHandler handler(request, response, config_[tgt_index]);
  handler.process();
}
