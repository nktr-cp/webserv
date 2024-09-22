#ifndef CGI_HPP
#define CGI_HPP

#include <sys/wait.h>
#include <unistd.h>

#include "http_request.hpp"
#include "http_response.hpp"
#include "location.hpp"
#include "utils.hpp"

#define BUFFER_SIZE 1024

class cgiMaster {
 private:
  void setEnvironment();
  void createPipes();
  void handleChildProcess();
  void handleParentProcess();

  const HttpRequest *request_;
  HttpResponse *response_;
  std::string cgiPath;
  std::map<std::string, std::string> env_;
  int inpipe_[2];
  int outpipe_[2];
  pid_t pid_;
  std::string output_;

  char **envToCArray();
  void generateHTTPHeader();

 public:
  cgiMaster(const HttpRequest *request, HttpResponse *response,
            const Location *location);
  ~cgiMaster();

  void execute();
};

#endif