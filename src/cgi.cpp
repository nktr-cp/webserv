#include "webserv.hpp"
#include "cgi.hpp"

cgiMaster::cgiMaster(const HttpRequest *request, HttpResponse *response, const Location *location)
    : request_(request), response_(response), cgiPath(location->getCgiPath()) {
  setEnvironment();
  createPipes();
}

cgiMaster::~cgiMaster() {
  close(inpipe_[0]);
  close(inpipe_[1]);
  close(outpipe_[0]);
  close(outpipe_[1]);
}

void cgiMaster::setEnvironment() {//TODO:ない可能性があるものの確認
  env_["REQUEST_METHOD"] = request_->getMethod();
  env_["REQUEST_URI"] = request_->getUri();
  env_["SERVER_SOFTWARE"] = "webserv";
  env_["SERVER_NAME"] = request_->getHostName();
  env_["SERVER_PORT"] = request_->getHostPort();
  env_["GATEWAY_INTERFACE"] = "CGI/1.1";
  env_["SERVER_PROTOCOL"] = HTTP_VERSION;
  env_["SCRIPT_FILENAME"] = cgiPath;
  env_["SCRIPT_NAME"] = cgiPath;
  env_["CONTENT_LENGTH"] = std::to_string(request_->getBody().length());
  env_["PATH_INFO"] = request_->getUri();
  env_["PATH_TRANSLATED"] = request_->getUri();
  // env_["QUERY_STRING"] = request_->getQuery();//クエリストリングをそのまま？
}

void cgiMaster::createPipes() {
  if (pipe(inpipe_) == -1 || pipe(outpipe_) == -1) {
    throw SysCallFailed("pipe");
  }
}

void cgiMaster::execute() {
  pid_ = fork();
  if (pid_ == -1) {
    throw SysCallFailed("fork");
  }

  if (pid_ == 0) {
    handleChildProcess();
  } else {
    handleParentProcess();
    generateHTTPHeader();
    size_t rnrn = output_.find("\r\n\r\n");
    size_t nn = output_.find("\n\n");
    if (rnrn == std::string::npos && nn == std::string::npos)
      return;
    size_t pos = rnrn == std::string::npos ? nn + 2 : rnrn + 4;
    response_->setBody(output_.substr(pos));
  }
}

char **cgiMaster::envToCArray() {
  char **envp = NULL;
  try {
    envp = new char*[env_.size() + 1];
    int i = 0;
    for (std::map<std::string, std::string>::iterator it = env_.begin(); it != env_.end(); ++it) {
      std::string env = it->first + "=" + it->second;
      envp[i] = new char[env.length() + 1];
      strcpy(envp[i], env.c_str());
      ++i;
    }
    envp[i] = NULL;
  } catch (std::bad_alloc& ba) {
    for (int j = 0; j < env_.size(); ++j) {
      delete[] envp[j];
    }
    delete[] envp;
    throw SysCallFailed("new");
  }
  return envp;
}

void cgiMaster::handleChildProcess() {
  close(inpipe_[1]);
  close(outpipe_[0]);
  dup2(inpipe_[0], STDIN_FILENO);
  dup2(outpipe_[1], STDOUT_FILENO);
  close(inpipe_[0]);
  close(outpipe_[1]);
  char cwd[PATH_MAX];
  if (getcwd(cwd, sizeof(cwd)) == NULL)
    throw SysCallFailed("getcwd");
  std::string fullCgiPath = std::string(cwd) + cgiPath;

  //envvar
  char **envp = envToCArray();
  execve(fullCgiPath.c_str(), NULL, envp);
  throw SysCallFailed("execve");
}

void cgiMaster::handleParentProcess() {
  close(inpipe_[0]);
  close(outpipe_[1]);
  std::string body = request_->getBody();
  write(inpipe_[1], body.c_str(), body.length());
  close(inpipe_[1]);

  char buffer[BUFFER_SIZE];
  ssize_t n;
  while ((n = read(outpipe_[0], buffer, BUFFER_SIZE)) > 0) {
    output_.append(buffer, n);
  }
  close(outpipe_[0]);
  int status;
  waitpid(pid_, &status, 0);
  if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
    throw http::responseStatusException(INTERNAL_SERVER_ERROR);
  }
}

void cgiMaster::generateHTTPHeader() {
  std::string line;
  std::istringstream iss(output_);
  while (std::getline(iss, line)) {
    if (line.empty()) {
      break;
    }
    size_t pos = line.find(":");
    if (pos == std::string::npos) {
      continue;
    }
    std::string key = line.substr(0, pos);
    std::string value = line.substr(pos + 1);
    if (key == "Status") {
      std::istringstream iss(value);
      std::string status;
      iss >> status;
      unsigned int range[] = {100, 599};//本来はチェックしない、すなわち"Status: 999 GOMI"でも通る
      try {
        response_->setStatus(static_cast<HttpStatus>(ft::stoui(status, range)));
      } catch (const std::exception& e) {
        throw http::responseStatusException(BAD_GATEWAY);
      }
      if (!(iss >> status) || status != http::statusToString(response_->getStatus()))
        throw http::responseStatusException(BAD_GATEWAY);
    } else
      response_->setHeader(key, value);
  }
}
