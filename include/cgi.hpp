#ifndef CGI_HPP
#define CGI_HPP

#include <sys/wait.h>
#include <unistd.h>

#include "http_request.hpp"
#include "http_response.hpp"
#include "location.hpp"
#include "utils.hpp"

#define BUFFER_SIZE 1024

class CgiMaster {
 public:
  const static std::string kCgiResponseHeader;
  
 private:
  void setEnvironment();
  void createPipes();
  void handleChildProcess(int outpipe);
  void handleParentProcess();

  const HttpRequest *request_;
  std::string cgiPath;
  std::map<std::string, std::string> env_;
  int inpipe_[2];
  pid_t pid_;

  char **envToCArray();

 public:
  CgiMaster(const HttpRequest *request, const Location *location);
  ~CgiMaster();

  void execute(int inpipe);
  static std::string CgiResponseFormatter(std::string response);
};

#endif