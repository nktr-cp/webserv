#include "server.hpp"

ServerManager::ServerManager(const std::string& file_path) {
  Config config(file_path);
  std::vector<ServerConfig> server_configs = config.getServerConfigs();
  std::vector<ServerConfig>::iterator it = server_configs.begin();
  for (; it != server_configs.end(); ++it) {
    servers_.push_back(Server(*it));
  }
}

// サーバのソケットをイベントキューに登録する
// クライアントからの接続を検知できるようにする
void ServerManager::register_fd() {
  if ((kq_ = kqueue()) == -1) {
    throw SysCallFailed();
  }

  for (size_t i = 0; i < this->size(); i++) {
    struct kevent event;
    SocketPair sp =
        std::make_pair(servers_[i].getType(), servers_[i].getSockfd());
    EV_SET(&event, servers_[i].getSockfd(), EVFILT_READ, EV_ADD | EV_ENABLE, 0,
           0, reinterpret_cast<void*>(&sp));
    changes_.push_back(event);
  }
}

int ServerManager::find_server_index(int sock_fd) {
  for (size_t i = 0; i < servers_.size(); i++) {
    if (servers_[i].getSockfd() == sock_fd) {
      return i;
    }
  }
  return -1;
}

void ServerManager::handle_server(SocketPair sp) {
  struct sockaddr_storage client_addr;
  socklen_t addr_size = sizeof(client_addr);

  int client_sockfd = accept(
      sp.second, reinterpret_cast<struct sockaddr*>(&client_addr), &addr_size);
  if (client_sockfd == -1) {
    throw SysCallFailed();
  }

  if (fcntl(client_sockfd, F_SETFL, O_NONBLOCK) == -1) {
    throw SysCallFailed();
  }

  int index = find_server_index(sp.second);
  if (index == -1) {
    throw std::runtime_error("Server not found");
  }
  servers_[index].addClient(client_sockfd);
}

void ServerManager::handle_client(SocketPair sp) {
  // procedure
  (void)sp;
}

void ServerManager::event_loop() {
  struct kevent events[ServerManager::MAX_EVENTS];

  int nev = kevent(kq_, &changes_[0], changes_.size(), events,
                   ServerManager::MAX_EVENTS, NULL);
  if (nev < 0) {
    throw SysCallFailed();
  }

  for (int i = 0; i < nev; i++) {
    if (events[i].flags & EV_ERROR) {
      throw SysCallFailed();
    } else if (events[i].filter == EVFILT_READ) {
      SocketPair sp = *reinterpret_cast<SocketPair*>(events[i].udata);
      // クライアントの用意
      if (sp.first == kServer) {
        handle_server(sp);
      } else {
        assert(sp.first == kClient);
        handle_client(sp);
      }
    }
  }
}

void ServerManager::run() {
  register_fd();
  while (true) {
    void event_loop();
  }
}

void Server::addClient(int client_fd) {
  this->clients_.push_back(Client(client_fd));
}

std::vector<Server> ServerManager::getServers() const { return servers_; }

size_t ServerManager::size() const { return servers_.size(); }
