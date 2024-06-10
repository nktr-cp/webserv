#include "client.hpp"

Client::Client(int sock_fd) : sock_fd_(sock_fd) {}

int Client::getSockfd() const { return sock_fd_; }
