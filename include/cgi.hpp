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
  enum interpreter
  {
    PYTHON,
    SH,
    BIN,
    UNKNOWN
  };
  const HttpRequest *request_;
  std::map<std::string, std::string> env_;
  int inpipe_[2];
  int outpipe_[2];
  std::string cgiPath_;
  interpreter interpreter_;

  void setEnvironment();
  void createPipes();
  void identifyInterpreter();
  char **envToCArray();

public:
  CgiMaster(const HttpRequest *request, const Location *location);
  ~CgiMaster();
  std::pair<pid_t, int> execute();
  void handleChildProcess();
  void handleParentProcess();
  static HttpResponse convertCgiResponse(const std::string &cgiResponse);
};

#endif