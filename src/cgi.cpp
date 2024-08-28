#include "webserv.hpp"
#include "cgi.hpp"

cgiMaster::cgiMaster(const HttpRequest& request, int client_fd) : request_(request), client_fd_(client_fd) {
  setEnvironment();
  createPipes();
}

cgiMaster::~cgiMaster() {
  close(inpipe_[0]);
  close(inpipe_[1]);
  close(outpipe_[0]);
  close(outpipe_[1]);
}

void cgiMaster::setEnvironment() {
  env_["REQUEST_METHOD"] = request_.getMethod();
  env_["REQUEST_URI"] = request_.getUri();
  env_["SERVER_SOFTWARE"] = "webserv";
  env_["SERVER_NAME"] = request_.getHostName();
  env_["SERVER_PORT"] = request_.getHostPort();
  // TODO: set other environment variables
}

void cgiMaster::createPipes() {
  if (pipe(inpipe_) == -1 || pipe(outpipe_) == -1) {
    throw SysCallFailed();
  }
}

void cgiMaster::execute() {
  cgi_pid_ = fork();
  if (cgi_pid_ == -1) {
    throw SysCallFailed();
  }

  if (cgi_pid_ == 0) {
    handleChildProcess();
  } else {
    handleParentProcess();
    send(client_fd_, cgi_output_.c_str(), cgi_output_.length(), 0);//雑すぎる可能性
  }
}

void cgiMaster::handleChildProcess() {
  close(inpipe_[1]);
  close(outpipe_[0]);
  dup2(inpipe_[0], STDIN_FILENO);
  dup2(outpipe_[1], STDOUT_FILENO);
  close(inpipe_[0]);
  close(outpipe_[1]);

  //envvar
  std::vector<std::string> env;
  for (std::map<std::string, std::string>::const_iterator it = env_.begin(); it != env_.end(); it++) {
    env.push_back(it->first + "=" + it->second);
  }
  env.push_back(NULL);
  // TODO: Identify GCI path and execute it
}

void cgiMaster::handleParentProcess() {
  close(inpipe_[0]);
  close(outpipe_[1]);
  std::string body = request_.getBody();
  write(inpipe_[1], body.c_str(), body.length());
  close(inpipe_[1]);

  char buffer[BUFFER_SIZE];
  ssize_t n;
  while ((n = read(outpipe_[0], buffer, BUFFER_SIZE)) > 0) {
    cgi_output_.append(buffer, n);
  }
  close(outpipe_[0]);
  int status;
  waitpid(cgi_pid_, &status, 0);
}

