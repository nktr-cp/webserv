#ifndef SERVER_HPP_
#define SERVER_HPP_

#include "cache.hpp"
#include "client.hpp"
#include "config.hpp"
#include "io_manager.hpp"
#include "request.hpp"
#include "response.hpp"
#include "router.hpp"

class Server {
 private:
  Cache cache_;
  Config config_;
  std::vector<Client> clients_;
  IOManager io_manager_;
  Router router_;
  Response response_;
  Request request_;

 public:
  Server(const std::string& file_path);

  void start();
  void stop();
  void acceptClient();
};

#endif  // SERVER_HPP_
