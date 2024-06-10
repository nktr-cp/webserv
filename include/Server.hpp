#ifndef SERVER_HPP_
#define SERVER_HPP_

#include <fcntl.h>
#include <netdb.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cassert>

#include "cache.hpp"
#include "client.hpp"
#include "config.hpp"
#include "io_manager.hpp"
#include "request.hpp"
#include "response.hpp"
#include "router.hpp"

class Server {
 private:
  ServerConfig config_;
  std::vector<Client> clients_;
  int sock_fd_;
  SocketType type_;

 public:
  Server(ServerConfig config);

  void addClient(int client_fd);

  int getSockfd() const;
  SocketType getType() const;
  std::vector<Client> getClients() const;
};

class ServerManager {
 private:
  static const int MAX_EVENTS = 10;

  int kq_;
  std::vector<struct kevent> changes_;
  std::vector<Server> servers_;

  int find_server_index(int sock_fd);
  void register_fd();
  void handle_server(SocketPair sp);
  void handle_client(SocketPair sp);
  void event_loop();

 public:
  ServerManager(const std::string& file_path);

  void run();

  std::vector<Server> getServers() const;
  size_t size() const;
};

#endif  // SERVER_HPP_
