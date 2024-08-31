#include "webserv.hpp"
#include "cgi.hpp"

cgiMaster::cgiMaster(const HttpRequest& request, int client_fd, const Location *location) : request_(request), clientFd_(client_fd), cgiPath(location->getCgiPath()) {
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
  pid_ = fork();
  if (pid_ == -1) {
    throw SysCallFailed();
  }

  if (pid_ == 0) {
    handleChildProcess();
  } else {
    handleParentProcess();
    send(clientFd_, output_.c_str(), output_.length(), 0);//雑すぎる可能性、おそらく情報を追加する必要あり
  }
}

char ** cgiMaster::envToCArray() {
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
    throw SysCallFailed();
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

  //envvar
  char **envp = envToCArray();
  execve(cgiPath.c_str(), NULL, envp);
  throw SysCallFailed();
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
    output_.append(buffer, n);
  }
  close(outpipe_[0]);
  int status;
  waitpid(pid_, &status, 0);
}

const Location* Server::requestLocationMatch(const HttpRequest &request) const {
    const std::vector<Location>& locations = config_.front().getLocations();
    size_t max_count = 0;
    const Location* matchedLocation = NULL;
    std::string uri = request.getUri();

    if (uri[uri.size() - 1] != '/') {
        uri += "/";
    }

    for (size_t i = 0; i < locations.size(); ++i) {
        const std::string& path = locations[i].getName();
        size_t cur_count = 0;
        while (cur_count < path.size() && cur_count < uri.size() && path[cur_count] == uri[cur_count]) {
            if (path[cur_count] == '/' && cur_count > max_count) {
                max_count = cur_count;
                matchedLocation = &locations[i];
            }
            ++cur_count;
        }
    }

    return matchedLocation;//このまま出したらチート
}


