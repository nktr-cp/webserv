#ifndef CGI_HPP
#define CGI_HPP

#include "http_request.hpp"
#include "http_response.hpp"
#include "utils.hpp"

#define BUFFER_SIZE 1024

class cgiMaster {
private:
  void setEnvironment();
  void createPipes();
  void handleChildProcess();
  void handleParentProcess();

  const HttpRequest& request_;
  int client_fd_;
  std::map<std::string, std::string> env_;
  int inpipe_[2];
  int outpipe_[2];
  pid_t cgi_pid_;
  std::string cgi_output_;

public:
  cgiMaster(const HttpRequest& request, int client_fd);
  ~cgiMaster();

  void execute();
};

#endif