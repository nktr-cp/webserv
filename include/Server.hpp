#ifndef SERVER_HPP_
#define SERVER_HPP_

#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/event.h>

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
  int sock_fd_;

 public:
  Server(const std::string& file_path);

  void start();
  void stop();
  void acceptClient();

  int getSockfd() const;
};

class ServerManager {
  private:
    fd_set readfds_, writefds_;
    std::vector<Server> servers_;

  static void set_select();
  static void accept_clients();
  static void read_requests();

  public:
    void add_server(Server server);
};

#endif  // SERVER_HPP_
