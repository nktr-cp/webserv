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

  fd_v_pid_.clear();
  fd_v_client_.clear();
  connections_.clear();
  response_buffers_.clear();
}

void Webserv::createServerSockets() {
  for (size_t i = 0; i < servers_.size(); i++) {
    servers_[i].createSocket();
  }
}

void Webserv::run() {
  createServerSockets();

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
          HttpResponse error;
          
          if (fd_v_pid_.count(fd)) {
            int client_fd = fd_v_client_[fd];
            fd_v_client_.erase(fd);
            fd_v_pid_.erase(fd);
            closeConnection(fd);

            error.setStatus(BAD_GATEWAY);
            std::string res_str = error.encode();
            send(client_fd, res_str.c_str(), res_str.length(), 0);
            std::fill(buffer_.begin(), buffer_.end(), 0);
          } else {
            error.setStatus(INTERNAL_SERVER_ERROR);
            sendResponse(fd, error, false);
          }
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
        if (events_[i].events & EPOLLIN) {
          try {
            // outgoing CGI response if fd is already registered in rfdtopid_
            if (fd_v_pid_.count(fd)) {
              //read CGI response
              char buf[1024];
              std::string cgiResponse;
              ssize_t read_bytes;

              while ((read_bytes = read(fd, buf, 1024)) > 0) {
                cgiResponse.append(buf, read_bytes);
              }
              //TODO: may need to check if the response if empty
              // convert CGI response to HttpResponse
              HttpResponse response = CgiMaster::convertCgiResponse(cgiResponse);
              // delete pid/cgi fd/client fd from fd_v_pid_ and fd_v_client_
              pid_t pid = fd_v_pid_[fd];
              int client_fd = fd_v_client_[fd];
              fd_v_pid_.erase(fd);
              fd_v_client_.erase(fd);
              closeConnection(fd);

              int status;
              waitpid(pid, &status, 0);
              
              bool keepAlive = fd_v_kp_[client_fd];
              fd_v_kp_.erase(client_fd);
              if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                registerSendEvent(client_fd, response, keepAlive);
              } else {
                HttpResponse error;
                error.setStatus(INTERNAL_SERVER_ERROR);
                registerSendEvent(client_fd, error, keepAlive);
              }

            }
            else
            {
              handleClientData(fd);
            }
          } catch (const SysCallFailed &e) {
          }
        } else if (events_[i].events & EPOLLOUT) {
          try {
            if (response_buffers_.count(fd)) {
              std::pair<HttpResponse, bool> response_pair = response_buffers_[fd];
              HttpResponse response = response_pair.first;
              bool keepAlive = response_pair.second;
              sendResponse(fd, response, keepAlive);
              response_buffers_.erase(fd);
            }
          } catch (const SysCallFailed &e) {
          }
        }
      }
    }
  }
}

void Webserv::handleTimeout() {
  time_t currentTime = time(NULL);

  for (std::map<int, time_t>::iterator it = connections_.begin();
       it != connections_.end(); it++) {
    // timeout for CGI
    if (fd_v_client_.count(it->first) && currentTime > it->second + kTimeoutSec) {
      int child_fd = it->first;
      int client_fd = fd_v_client_[it->first];
      HttpResponse response;
      response.setStatus(GATEWAY_TIMEOUT);

      bool keepAlive = fd_v_kp_[client_fd];
      registerSendEvent(client_fd, response, keepAlive);
      pid_t pid = fd_v_pid_[child_fd];
      fd_v_pid_.erase(child_fd);
      fd_v_client_.erase(child_fd);
      fd_v_pid_.erase(child_fd);
      closeConnection(child_fd);
      kill(pid, SIGKILL);
      int status;
      waitpid(pid, &status, 0);
    }
  }
}

void Webserv::closeConnection(int sock_fd) {
  if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, sock_fd, NULL) == -1) {
    throw SysCallFailed("epoll_ctl delete");
  }
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

  struct epoll_event ev;
  ev.events = EPOLLIN;
  ev.data.fd = client_fd;
  if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, client_fd, &ev) == -1) {
    close(client_fd);
    throw SysCallFailed("epoll_ctl add");
  }
  connections_[client_fd] = time(NULL);
}

void Webserv::handleClientData(int client_fd) {
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
      closeConnection(client_fd);
      requests.erase(client_fd);
      return;
    }

    if (total_bytes > HttpRequest::kMaxPayloadSize - recv_bytes) {
      response.setStatus(PAYLOAD_TOO_LARGE);
      registerSendEvent(client_fd, response, request.keepAlive);
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
    registerSendEvent(client_fd, response, request.keepAlive);
    requests.erase(client_fd);
    return;
  } catch (const std::exception &e) {
    response.setStatus(INTERNAL_SERVER_ERROR);
    registerSendEvent(client_fd, response, request.keepAlive);
    requests.erase(client_fd);
    return;
  }

  if (request.progress != HttpRequest::DONE) return ;
  // 該当するサーバーを探してリクエストを処理
  std::string port = request.getHostPort();
  bool server_found = false;
  for (size_t i = 0; i < servers_.size(); i++) {
    if (servers_[i].getConfig().front().getPort() == port) {
      server_found = true;
      Server &server = servers_[i];
      RequestHandler rh = server.getHander(request, response);
      // CGIリクエストの場合
      if (rh.isCGIRequest()) {
        std::pair<pid_t, int> pid_fd;
        try {
          CgiMaster cgi(&request, rh.getLocation());
          pid_fd = cgi.execute();
        } catch (const SysCallFailed &e) {
          response.setStatus(INTERNAL_SERVER_ERROR);
          registerSendEvent(client_fd, response, request.keepAlive);
          requests.erase(client_fd);
          return;
        } catch (const http::responseStatusException &e) {
          response.setStatus(e.getStatus());
          registerSendEvent(client_fd, response, request.keepAlive);
          requests.erase(client_fd);
          return;
        }
        //error if pid is -1
        if (pid_fd.first == -1) {
          if (errno == ENOENT) {
            response.setStatus(NOT_FOUND);
          } else if (errno == EACCES) {
            response.setStatus(FORBIDDEN);
          } else {
            response.setStatus(INTERNAL_SERVER_ERROR);
          }
          registerSendEvent(client_fd, response, request.keepAlive);
          requests.erase(client_fd);
          return;
        }
        fd_v_pid_[pid_fd.second] = pid_fd.first;
        fd_v_client_[pid_fd.second] = client_fd;
        fd_v_kp_[client_fd] = request.keepAlive;
        //register CGI fd to epoll
        connections_[pid_fd.second] = time(NULL);
        struct epoll_event ev;
        ev.events = EPOLLIN;
        ev.data.fd = pid_fd.second;
        if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, pid_fd.second, &ev) == -1) {
          throw SysCallFailed("epoll_ctl add");
        }
        requests.erase(client_fd);
        return;
      } else {
        server.handleRequest(request, response, rh);
      }
      break;
    }
  }
  if (!server_found) {
    response.setStatus(NOT_FOUND);
  }

  registerSendEvent(client_fd, response, request.keepAlive);
  requests.erase(client_fd);
}

void Webserv::registerSendEvent(int client_fd, const HttpResponse &response, bool keepAlive) {
  response_buffers_[client_fd] = std::make_pair(response, keepAlive);

  struct epoll_event ev;
  ev.events = EPOLLOUT;
  ev.data.fd = client_fd;
  if (epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, client_fd, &ev) == -1) {
    throw SysCallFailed("epoll_ctl mod");
  }
}

void Webserv::sendResponse(const int client_fd, const HttpResponse &response, bool keepAlive) {
  std::string res_str = response.encode();
  send(client_fd, res_str.c_str(), res_str.length(), 0);
  std::fill(buffer_.begin(), buffer_.end(), 0);
  if (!keepAlive) {
    closeConnection(client_fd);
  } else {
    connections_[client_fd] = time(NULL);
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = client_fd;
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, client_fd, &ev) == -1) {
        throw SysCallFailed("epoll_ctl mod");
    }
  }
}
