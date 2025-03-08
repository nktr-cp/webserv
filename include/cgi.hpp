#ifndef CGI_HPP
#define CGI_HPP

#include <sys/wait.h>
#include <unistd.h>

#include "http_request.hpp"
#include "http_response.hpp"
#include "location.hpp"
#include "utils.hpp"

#define BUFFER_SIZE 1024

class CgiMaster
{
private:
  const HttpRequest *request_;
  const Location *location_;
  std::map<std::string, std::string> env_;
  int inpipe_[2];
  int outpipe_[2];
  std::string cgiPath_;

  void setEnvironment();
  void createPipes();
  char **envToCArray();

public:
  CgiMaster(const HttpRequest *request, const Location *location);
  ~CgiMaster();
  std::pair<pid_t, int> execute();
  void handleChildProcess();
  void handleParentProcess();
  HttpResponse convertCgiResponse(const std::string &cgiResponse);
};

#endif