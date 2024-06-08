#ifndef CLIENT_HPP_
#define CLIENT_HPP_

#include "request.hpp"
#include "response.hpp"

class Client {
 private:
  int socket_;
  Request request_;
  Response response_;

 public:
  Client(int);
};

#endif  // CLIENT_HPP_
