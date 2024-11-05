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
      puts("====================");
      std::cout << nev << " events" << std::endl;
      std::cout << "Pending " << pendingRequests_.size() << " requests"
                << std::endl;
      puts("--------------------");
#define D std::cout << __LINE__ << std::endl;
      int fd = events_[i].ident;

      bool isPendingRequest = false;
      int client = -1;
      for (std::map<int, std::set<int> >::const_iterator it =
               pendingRequests_.begin();
           it != pendingRequests_.end(); ++it) {
        if (it->second.find(fd) != it->second.end()) {
          isPendingRequest = true;
          client = it->first;
          break;
        }
      }
      D bool isServerSocket = false;
      for (size_t i = 0; i < servers_.size(); i++) {
        if (servers_[i].getServerFd() == fd) {
          isServerSocket = true;
          break;
        }
      }

      D if (!isPendingRequest && (events_[i].flags & EV_EOF)) {
        try {
          std::cout << "Closing" << std::endl;
          closeConnection(fd);
          std::cout << "Closed" << std::endl;
        } catch (const SysCallFailed &e) {
        }
      }
      D if (isServerSocket && !isPendingRequest) {
        D try {
          std::cout << "Connecting" << std::endl;
          handleNewConnection(fd);
          std::cout << "Connected" << std::endl;
          break;
        } catch (const SysCallFailed &e) {
        }
      }
      else {
        if (isPendingRequest) {
          D try {
            std::cout << "sending" << std::endl;
            sendResponse(client, fd, false);
            std::cout << "sent" << std::endl;
          } catch (const SysCallFailed &e) {
          }
        } else {
          D try {
            std::cout << "registering" << std::endl;
            registerClientRequest(fd);
            std::cout << "registered" << std::endl;
          } catch (const SysCallFailed &e) {
          }
        }
      }

      // if (events_[i].flags & EV_EOF) {
      //   puts("Closing");
      //   bool isPendingRequest = false;
      //   for (std::map<int, std::set<int> >::const_iterator it =
      //            pendingRequests_.begin();
      //        it != pendingRequests_.end(); ++it) {
      //     if (it->second.find(fd) != it->second.end()) {
      //       std::cout << "this fd is pending request, aborting" << std::endl;
      //       isPendingRequest = true;
      //     }
      //   }
      //   if (!isPendingRequest) {
      //     try {
      //       closeConnection(fd);
      //     } catch (const SysCallFailed &e) {
      //     }
      //   }
      //   // continue;
      // }

      // bool isServerSocket = false;
      // for (size_t i = 0; i < servers_.size(); i++) {
      //   if (servers_[i].getServerFd() == fd) {
      //     isServerSocket = true;
      //     try {
      //       handleNewConnection(fd);
      //     } catch (const SysCallFailed &e) {
      //     }
      //     puts("Connected");
      //     break;
      //   }
      // }

      // if (!isServerSocket) {
      //   puts("Client: request registration or response");
      //   try {
      //     bool isPendingRequest = false;
      //     for (std::map<int, std::set<int> >::const_iterator it =
      //              pendingRequests_.begin();
      //          it != pendingRequests_.end(); ++it) {
      //       if (it->second.find(fd) != it->second.end()) {
      //         std::cout << "sending" << std::endl;
      //         sendResponse(it->first, fd, false);
      //         std::cout << "sent" << std::endl;
      //         pendingRequests_.erase(it->first);
      //         isPendingRequest = true;
      //         break;
      //       }
      //     }
      //     if (!isPendingRequest) {
      //       std::cout << "registering" << std::endl;
      //       registerClientRequest(fd);
      //       std::cout << "registered" << std::endl;
      //     }
      //   } catch (const SysCallFailed &e) {
      //   }
      // }
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
          registerClientRequest(fd);
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

void Webserv::registerClientRequest(int client_fd) {
  static std::map<int, HttpRequest> requests;
  std::string request_data;
  ssize_t recv_bytes = 0;
  size_t total_bytes = 0;

  if (requests.find(client_fd) == requests.end()) {
    requests[client_fd] = HttpRequest();
  }
  HttpRequest &request = requests[client_fd];
  HttpResponse response;

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
      sendErrorResponse(client_fd, response, request.keepAlive);
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
    sendErrorResponse(client_fd, response, request.keepAlive);
    requests.erase(client_fd);
    return;
  } catch (const std::exception &e) {
    response.setStatus(INTERNAL_SERVER_ERROR);
    sendErrorResponse(client_fd, response, request.keepAlive);
    requests.erase(client_fd);
    return;
  }

  if (request.progress != HttpRequest::DONE) return;

  // 該当するサーバーを探してリクエストを処理
  std::string port = request.getHostPort();
  bool server_found = false;
  int files[2];
  for (size_t i = 0; i < servers_.size(); i++) {
    if (servers_[i].getConfig().front().getPort() == port) {
      server_found = true;
      Server &server = servers_[i];
      if (pipe(files) == -1) {
        response.setStatus(INTERNAL_SERVER_ERROR);
        sendErrorResponse(client_fd, response, request.keepAlive);
        requests.erase(client_fd);
        throw SysCallFailed("pipe");
      }
      pendingRequests_[client_fd].insert(files[0]);
      struct kevent ev;
      EV_SET(&ev, files[0], EVFILT_READ, EV_ADD, 0, 0, NULL);
      if (kevent(kq_, &ev, 1, NULL, 0, NULL) == -1) {
        throw SysCallFailed("kevent add");
      }

      server.handleRequest(request, response, files[1]);
      break;
    }
  }
  if (!server_found) {
    close(files[0]);
    close(files[1]);
    response.setStatus(NOT_FOUND);
  }
  // レスポンスをクライアントに送信
  // sendErrorResponse(client_fd, response, request.keepAlive);
  // requests.erase(client_fd);
}

void Webserv::sendResponse(const int client_fd, const int inpipe,
                           bool keepAlive) {
  std::string response_data;
  ssize_t recv_bytes = 0;
  size_t total_bytes = 0;
  while (true) {
    recv_bytes = read(inpipe, &buffer_[0], kBufferSize);
    std::cerr << recv_bytes << std::endl;
    if (recv_bytes < 0) {
      break;
    } else if (recv_bytes == 0) {
      // Server closed connection
      close(client_fd);
      close(inpipe);
      return;
    }

    // use the same value as HttpRequest::kMaxPayloadSize
    if (total_bytes > HttpRequest::kMaxPayloadSize - recv_bytes) {
      close(client_fd);
      close(inpipe);
      return;
    }
    total_bytes += recv_bytes;
    // Append the received chunk to the response_data string
    response_data.append(buffer_.data(), recv_bytes);
    std::cout << response_data << std::endl;
  }
  send(client_fd, response_data.c_str(), response_data.length(), 0);
  std::fill(buffer_.begin(), buffer_.end(), 0);
  close(inpipe);
  pendingRequests_[client_fd].erase(inpipe);
  if (!keepAlive) {
    closeConnection(client_fd);
  }
}

void Webserv::sendErrorResponse(const int client_fd,
                                const HttpResponse &response, bool keepAlive) {
  std::string res_str = response.encode();
  std::cout << res_str << std::endl;
  send(client_fd, res_str.c_str(), res_str.length(), 0);
  std::fill(buffer_.begin(), buffer_.end(), 0);
  if (!keepAlive) {
    closeConnection(client_fd);
  }
}
