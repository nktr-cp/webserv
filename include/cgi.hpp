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
  int clientFd_;
  std::string cgiPath;
  std::map<std::string, std::string> env_;
  int inpipe_[2];
  int outpipe_[2];
  pid_t pid_;
  std::string output_;

  char** envToCArray();

public:
  cgiMaster(const HttpRequest& request, int client_fd, const Location *location);
  ~cgiMaster();

  void execute();
};

#endif