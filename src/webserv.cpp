#include "webserv.hpp"

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
    int nev = kevent(kq_, NULL, 0, &events_[0], kMaxEvents, NULL);
    if (nev < 0) {
      throw SysCallFailed("kevent");
    }

    for (int i = 0; i < nev; i++) {
      int fd = events_[i].ident;

      bool isServerSocket = false;
      for (size_t i = 0; i < servers_.size(); i++) {
        if (servers_[i].getServerFd() == fd) {
          isServerSocket = true;
          handleNewConnection(fd);
          break;
        }
      }

      if (!isServerSocket) {
        try {
          handleClientData(fd);
        } catch (const SysCallFailed &e) {
          close(fd);
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
  ev.events = EPOLLIN;
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
    int nev = epoll_wait(epoll_fd_, &events_[0], kMaxEvents, -1);
    if (nev < 0) {
      throw SysCallFailed("epoll_wait");
    }

    for (size_t i = 0; i < nev; i++) {
      int fd = events_[i].data.fd;

      bool isServerSocket = false;
      for (size_t i = 0; i < servers_.size(); i++) {
        if (servers_[i].getServerFd() == fd) {
          isServerSocket = true;
          handleNewConnection(fd);
          break;
        }
      }

      if (!isServerSocket) {
        try {
          handleClientData(fd);
        } catch (const SysCallFailed &e) {
          close(fd);
        }
      }
    }
  }
#endif
}

void Webserv::handleNewConnection(int server_fd) {
  struct sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);
  int client_fd =
      accept(server_fd, reinterpret_cast<struct sockaddr *>(&client_addr),
             &client_len);
  if (client_fd == -1) {
    if (errno != EWOULDBLOCK && errno != EAGAIN) {
      throw SysCallFailed("accept");
    }
    return;
  }

  int flags = fcntl(client_fd, F_GETFL, 0);
  fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

#ifdef __APPLE__
  struct kevent ev;
  EV_SET(&ev, client_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
  if (kevent(kq_, &ev, 1, NULL, 0, NULL) == -1) {
    close(client_fd);
    throw SysCallFailed("kevent add");
  }
#elif __linux__
  struct epoll_event ev;
  ev.events = EPOLLIN;
  ev.data.fd = client_fd;
  if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, client_fd, &ev) == -1) {
    close(client_fd);
    throw SysCallFailed("epoll_ctl add");
  }
#endif
}

void Webserv::handleClientData(int client_fd) {
  ssize_t recv_bytes = recv(client_fd, &buffer_[0], kBufferSize, 0);
  if (recv_bytes <= 0) {
    if (recv_bytes < 0) {
      throw SysCallFailed("recv");
    }
    close(client_fd);
    return;
  }
  HttpRequest request;
  HttpResponse response;

  // リクエストをパース
  try {
    request = HttpRequest(buffer_.data());
  } catch (const http::responseStatusException &e) {
    response.setStatus(e.getStatus());
    sendResponse(client_fd, response);
    return;
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
}

void Webserv::sendResponse(const int client_fd, const HttpResponse &response) {
  std::string res_str = response.encode();
  send(client_fd, res_str.c_str(), res_str.length(), 0);
  std::fill(buffer_.begin(), buffer_.end(), 0);
}
