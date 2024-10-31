#include "webserv.hpp"

Webserv::Webserv() {}

Webserv::Webserv(const std::string &configFile) {
  Config config(configFile);

  std::map<std::string, std::vector<ServerConfig> > portToConfigs;

  for (size_t i = 0; i < config.getServerConfigs().size(); i++) {
    ServerConfig serverConfig = config.getServerConfigs()[i];
    portToConfigs[serverConfig.getPort()].push_back(serverConfig);
  }

  for (std::map<std::string, std::vector<ServerConfig> >::iterator it =
           portToConfigs.begin();
       it != portToConfigs.end(); it++) {
    servers_.push_back(Server(it->second));
  }
}

void Webserv::createServerSockets() {
  for (size_t i = 0; i < servers_.size(); i++) {
    servers_[i].createSocket();
  }
}

void Webserv::run() {
  createServerSockets();

#ifdef __APPLE__
  kq_ = kqueue();
  if (kq_ == -1) {
    throw SysCallFailed("kqueue");
  }

  // Register server sockets with kqueue
  for (size_t i = 0; i < servers_.size(); i++) {
    struct kevent ev;
    EV_SET(&ev, servers_[i].getServerFd(), EVFILT_READ, EV_ADD, 0, 0, NULL);
    if (kevent(kq_, &ev, 1, NULL, 0, NULL) == -1) {
      throw SysCallFailed("kevent add");
    }
  }

  events_.resize(kMaxEvents);
  buffer_.resize(kBufferSize);

  // Event loop for kqueue
  while (true) {
    struct timespec timeout = {kTimeoutSec, 0};

    int nev;
    try {
      nev = kevent(kq_, NULL, 0, &events_[0], kMaxEvents, &timeout);
      if (nev < 0) {
        throw SysCallFailed("kevent");
      } else if (nev == 0) {
        handleTimeout();
      }
    } catch (const SysCallFailed &e) {
      continue;
    }

    for (int i = 0; i < nev; i++) {
      int fd = events_[i].ident;

      if (events_[i].flags & EV_EOF) {
        try {
          closeConnection(fd);
        } catch (const SysCallFailed &e) {
        }
        continue;
      }

      bool isServerSocket = false;
      for (size_t i = 0; i < servers_.size(); i++) {
        if (servers_[i].getServerFd() == fd) {
          isServerSocket = true;
          try {
            handleNewConnection(fd);
          } catch (const SysCallFailed &e) {
          }
          break;
        }
      }

      if (!isServerSocket) {
        try {
          handleClientData(fd);
        } catch (const SysCallFailed &e) {
        }
      }
    }
  }
#elif __linux__
  epoll_fd_ = epoll_create1(0);
  if (epoll_fd_ == -1) {
    throw SysCallFailed("epoll_create1");
  }

  // Register server sockets with epoll
  struct epoll_event ev;
  ev.events = EPOLLIN | EPOLLET;
  for (size_t i = 0; i < servers_.size(); i++) {
    ev.data.fd = servers_[i].getServerFd();
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, servers_[i].getServerFd(), &ev) ==
        -1) {
      throw SysCallFailed("epoll_ctl add");
    }
  }

  events_.resize(kMaxEvents);
  buffer_.resize(kBufferSize);

  // Event loop for epoll
  while (true) {
    int nev =
        epoll_wait(epoll_fd_, &events_[0], kMaxEvents, kTimeoutSec * 1000);
    try {
      if (nev < 0) {
        throw SysCallFailed("epoll_wait");
      } else if (nev == 0) {
        handleTimeout();
      }
    } catch (const SysCallFailed &e) {
      continue;
    }

    for (int i = 0; i < nev; i++) {
      int fd = events_[i].data.fd;

      if (events_[i].events & (EPOLLHUP | EPOLLERR)) {
        try {
          closeConnection(fd);
        } catch (const SysCallFailed &e) {
        }
        continue;
      }

      bool isServerSocket = false;
      for (size_t i = 0; i < servers_.size(); i++) {
        if (servers_[i].getServerFd() == fd) {
          isServerSocket = true;
          try {
            handleNewConnection(fd);
          } catch (const SysCallFailed &e) {
          }
          break;
        }
      }

      if (!isServerSocket) {
        try {
          handleClientData(fd);
        } catch (const SysCallFailed &e) {
        }
      }
    }
  }
#endif
}

void Webserv::handleTimeout() {
  time_t currentTime = time(NULL);
  std::vector<int> connectionsToClose;

  for (std::map<int, time_t>::iterator it = connections_.begin();
       it != connections_.end(); it++) {
    if (currentTime - it->second > kTimeoutSec) {
      connectionsToClose.push_back(it->first);
    }
  }

  for (size_t i = 0; i < connectionsToClose.size(); i++) {
    closeConnection(connectionsToClose[i]);
  }
}

void Webserv::closeConnection(int sock_fd) {
#ifdef __APPLE__
  struct kevent ev;
  EV_SET(&ev, sock_fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
  if (kevent(kq_, &ev, 1, NULL, 0, NULL) == -1) {
    throw SysCallFailed("kevent delete");
  }
#elif __linux__
  if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, sock_fd, NULL) == -1) {
    throw SysCallFailed("epoll_ctl delete");
  }
#endif
  connections_.erase(sock_fd);
  // manより: If how is SHUT_RDWR, further sends and receives will be
  // disallowed.
  shutdown(sock_fd, SHUT_RDWR);
  close(sock_fd);
}

void Webserv::handleNewConnection(int server_fd) {
  struct sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);
  int client_fd =
      accept(server_fd, reinterpret_cast<struct sockaddr *>(&client_addr),
             &client_len);
  if (client_fd == -1) {
    throw SysCallFailed("accept");
  }

  int flags = fcntl(client_fd, F_GETFL, 0);
  fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

  int opt = 1;
  if (setsockopt(client_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) ==
      -1) {
    close(client_fd);
    throw SysCallFailed("setsockopt");
  }

#ifdef __APPLE__
  struct kevent ev;
  EV_SET(&ev, client_fd, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, NULL);
  if (kevent(kq_, &ev, 1, NULL, 0, NULL) == -1) {
    close(client_fd);
    throw SysCallFailed("kevent add");
  }
#elif __linux__
  struct epoll_event ev;
  ev.events = EPOLLIN | EPOLLET;
  ev.data.fd = client_fd;
  if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, client_fd, &ev) == -1) {
    close(client_fd);
    throw SysCallFailed("epoll_ctl add");
  }
#endif
  connections_[client_fd] = time(NULL);
}

void Webserv::handleClientData(int client_fd) {
  static std::map<int, HttpRequest> requests;
  std::string request_data;
  ssize_t recv_bytes = 0;
  size_t total_bytes = 0;
  HttpResponse response;

  if (requests.find(client_fd) == requests.end()) {
    requests[client_fd] = HttpRequest();
  }
  HttpRequest &request = requests[client_fd];

  // リクエストを受信
  while (true) {
    recv_bytes = recv(client_fd, &buffer_[0], kBufferSize, 0);

    if (recv_bytes < 0) {
      break;
    } else if (recv_bytes == 0) {
      // Client closed connection
      close(client_fd);
      requests.erase(client_fd);
      return;
    }

    if (total_bytes > HttpRequest::kMaxPayloadSize - recv_bytes) {
      response.setStatus(PAYLOAD_TOO_LARGE);
      sendResponse(client_fd, response);
      requests.erase(client_fd);
      return;
    }
    total_bytes += recv_bytes;
    // Append the received chunk to the request_data string
    request_data.append(buffer_.data(), recv_bytes);
  }

  // リクエストをパース
  try {
    request.parseRequest(request_data.c_str());
  } catch (const http::responseStatusException &e) {
    response.setStatus(e.getStatus());
    sendResponse(client_fd, response);
    requests.erase(client_fd);
    return;
  } catch (const std::exception &e) {
    response.setStatus(INTERNAL_SERVER_ERROR);
    sendResponse(client_fd, response);
    requests.erase(client_fd);
    return;
  }

  switch (request.progress) {
    case HttpRequest::HEADER:
      return;
    case HttpRequest::BODY:
      return;
    case HttpRequest::DONE:
      break;
  }

  // 該当するサーバーを探してリクエストを処理
  std::string port = request.getHostPort();
  bool server_found = false;
  for (size_t i = 0; i < servers_.size(); i++) {
    if (servers_[i].getConfig().front().getPort() == port) {
      server_found = true;
      Server &server = servers_[i];
      server.handleRequest(request, response);
      break;
    }
  }
  if (!server_found) {
    response.setStatus(NOT_FOUND);
  }
  // レスポンスをクライアントに送信
  sendResponse(client_fd, response);
  requests.erase(client_fd);
}

void Webserv::sendResponse(const int client_fd, const HttpResponse &response) {
  std::string res_str = response.encode();
  send(client_fd, res_str.c_str(), res_str.length(), 0);
  std::fill(buffer_.begin(), buffer_.end(), 0);
}
