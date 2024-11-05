#include "cgi.hpp"

#include "webserv.hpp"

CgiMaster::CgiMaster(const HttpRequest *request, const Location *location)
    : request_(request), cgiPath(location->getCgiPath()) {
  setEnvironment();
  createPipes();
}

CgiMaster::~CgiMaster() {
  close(inpipe_[0]);
  close(inpipe_[1]);
}

void CgiMaster::setEnvironment() {  // TODO:ない可能性があるものの確認
  env_["REQUEST_METHOD"] = http::methodToString(request_->getMethod());
  env_["REQUEST_URI"] = request_->getUri();
  env_["SERVER_SOFTWARE"] = VersionInfo::kProgramName;
  env_["SERVER_NAME"] = request_->getHostName();
  env_["SERVER_PORT"] = request_->getHostPort();
  env_["GATEWAY_INTERFACE"] = VersionInfo::kCgiVersion;
  env_["SERVER_PROTOCOL"] = VersionInfo::kHttpVersion;
  env_["SCRIPT_FILENAME"] = cgiPath;
  env_["SCRIPT_NAME"] = cgiPath;
  env_["CONTENT_LENGTH"] = ft::uitost(request_->getBody().length());
  env_["PATH_INFO"] = request_->getUri();
  env_["PATH_TRANSLATED"] = request_->getUri();
  env_["QUERY_STRING"] = request_->getQueryAsStr();
  env_["HTTP_COOKIE"] = request_->getHeader("Cookie");
}

void CgiMaster::createPipes() {
  if (pipe(inpipe_) == -1) {
    throw SysCallFailed("pipe");
  }
}

void CgiMaster::execute(int inpipe) {
  pid_ = fork();
  if (pid_ == -1) {
    throw SysCallFailed("fork");
  }

  if (pid_ == 0) {
    handleChildProcess(inpipe);
  } else {
    handleParentProcess();
  }
}

char **CgiMaster::envToCArray() {
  char **envp = NULL;
  try {
    envp = new char *[env_.size() + 1];
    int i = 0;
    for (std::map<std::string, std::string>::iterator it = env_.begin();
         it != env_.end(); ++it) {
      std::string env = it->first + "=" + it->second;
      envp[i] = new char[env.length() + 1];
      strcpy(envp[i], env.c_str());
      ++i;
    }
    envp[i] = NULL;
  } catch (std::bad_alloc &ba) {
    for (size_t j = 0; j < env_.size(); ++j) {
      delete[] envp[j];
    }
    delete[] envp;
    throw SysCallFailed("new");
  }
  return envp;
}

void CgiMaster::handleChildProcess(int outpipe) {
  close(inpipe_[1]);
  dup2(inpipe_[0], STDIN_FILENO);
  dup2(outpipe, STDOUT_FILENO);
  close(inpipe_[0]);
  close(outpipe);
  char cwd[PATH_MAX];
  std::string fullCgiPath;
  if (cgiPath[0] != '/') {
    if (getcwd(cwd, sizeof(cwd)) == NULL) throw SysCallFailed("getcwd");
    fullCgiPath = std::string(cwd) + "/" + cgiPath;
  } else
    fullCgiPath = cgiPath;

  // envvar
  char **envp = envToCArray();
  char *argv[] = {const_cast<char *>(fullCgiPath.c_str()), NULL};
  execve(fullCgiPath.c_str(), argv, envp);
  throw SysCallFailed("execve");
  std::exit(EXIT_FAILURE);
}

void CgiMaster::handleParentProcess() {
  close(inpipe_[0]);
  std::string body = request_->getBody();
  write(inpipe_[1], body.c_str(), body.length());
  close(inpipe_[1]);

  int status;
  waitpid(pid_, &status, WNOHANG);
  if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
    throw http::responseStatusException(INTERNAL_SERVER_ERROR);
  }
}
