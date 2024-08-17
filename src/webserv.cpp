#include "webserv.hpp"

Webserv::Webserv(const std::string &configFile) {
  (void)configFile;
  servers_.push_back(Server());
}

void Webserv::createServerSockets() {
  for (size_t i = 0; i < servers_.size(); i++) {
    servers_[i].createSocket();
  }
}

void Webserv::run() {
  createServerSockets();

  // Create kqueue
  int kq_ = kqueue();
  if (kq_ == -1) {
    throw std::runtime_error("kqueue failed");
  }

  // Register server sockets with kqueue
  for (size_t i = 0; i < servers_.size(); i++) {
    struct kevent ev;
    EV_SET(&ev, servers_[i].getServerFd(), EVFILT_READ, EV_ADD, 0, 0, NULL);
    if (kevent(kq_, &ev, 1, NULL, 0, NULL) == -1) {
      throw std::runtime_error("kevent add server failed");
    }
  }

  events_.resize(kMaxEvents);
  buffer_.resize(kBufferSize);

  // Event loop
  while (true) {
    int nev = kevent(kq_, NULL, 0, &events_[0], kMaxEvents, NULL);
    if (nev < 0) {
      throw std::runtime_error("kevent wait failed");
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
        handleClientData(fd);
      }
    }
  }
}

void Webserv::handleNewConnection(int server_fd) {
  struct sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);
  int client_fd =
      accept(server_fd, reinterpret_cast<struct sockaddr *>(&client_addr),
             &client_len);
  if (client_fd == -1) {
    if (errno != EWOULDBLOCK && errno != EAGAIN) {
      std::runtime_error("accept failed");
    }
    return;
  }

  int flags = fcntl(client_fd, F_GETFL, 0);
  fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

  struct kevent ev;
  EV_SET(&ev, client_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
  if (kevent(kq_, &ev, 1, NULL, 0, NULL) == -1) {
    close(client_fd);
    throw std::runtime_error("kevent add client failed");
  }
}

void Webserv::handleClientData(int client_fd) {
  ssize_t recv_bytes = recv(client_fd, &buffer_[0], kBufferSize, 0);
  // ssize_t read_bytes = read(client_fd, &buffer_[0], kBufferSize);
  if (recv_bytes <= 0) {
    if (recv_bytes < 0) {
      throw std::runtime_error("recv failed");
    }
    close(client_fd);
    return;
  }

  // MEMO: ここでリクエストをパース、レスポンスを生成する
  // 上の機能が実装できれば以下のコードは消してください
  std::string response =
      "HTTP/1.1 200 OK\r\n"
      "Content-Type: text/plain\r\n"
      "Content-Length: " +
      std::to_string(recv_bytes) +
      "\r\n"
      "\r\n";

  response.append(buffer_.begin(), buffer_.begin() + recv_bytes);
  send(client_fd, response.c_str(), response.length(), 0);
  std::fill(buffer_.begin(), buffer_.end(), 0);
}
