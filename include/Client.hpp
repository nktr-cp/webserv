#ifndef CLIENT_HPP_
#define CLIENT_HPP_

#include "request.hpp"
#include "response.hpp"

class Client {
 private:
  int sock_fd_;
  // Request request_;
  // Response response_;

 public:
  Client(int);

  int getSockfd() const;
};

#endif  // CLIENT_HPP_
