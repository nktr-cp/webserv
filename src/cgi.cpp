#include "cgi.hpp"
#include "webserv.hpp"

CgiMaster::CgiMaster(const HttpRequest *request, const Location *location)
    : request_(request), cgiPath_(location->getCgiPath())
{
  setEnvironment();
  createPipes();
  identifyInterpreter();
}

CgiMaster::~CgiMaster()
{
  close(inpipe_[0]);
  close(inpipe_[1]);
  close(outpipe_[1]);
}

void CgiMaster::setEnvironment() {
  env_["REQUEST_METHOD"] = http::methodToString(request_->getMethod());
  env_["REQUEST_URI"] = request_->getUri();
  env_["SERVER_SOFTWARE"] = VersionInfo::kProgramName;
  env_["SERVER_NAME"] = request_->getHostName();
  env_["SERVER_PORT"] = request_->getHostPort();
  env_["GATEWAY_INTERFACE"] = VersionInfo::kCgiVersion;
  env_["SERVER_PROTOCOL"] = VersionInfo::kHttpVersion;
  env_["SCRIPT_FILENAME"] = cgiPath_;
  env_["SCRIPT_NAME"] = cgiPath_;
  env_["CONTENT_LENGTH"] = ft::uitost(request_->getBody().length());
  env_["PATH_INFO"] = request_->getUri();
  env_["PATH_TRANSLATED"] = request_->getUri();
  env_["QUERY_STRING"] = request_->getQueryAsStr();
  env_["HTTP_COOKIE"] = request_->getHeader("Cookie"); 
}

void CgiMaster::createPipes()
{
  if (pipe(inpipe_) == -1 || pipe(outpipe_) == -1)
  {
    throw SysCallFailed("pipe");
  }
}

void CgiMaster::identifyInterpreter()
{
  size_t pos = cgiPath_.find_last_of('.');
  if (pos == std::string::npos)
  {
    interpreter_ = UNKNOWN;
    return;
  }
  std::string extension = cgiPath_.substr(pos + 1);
  if (extension == "py")
  {
    interpreter_ = PYTHON;
  }
  else if (extension == "sh")
  {
    interpreter_ = SH;
  }
  else
  {
    interpreter_ = UNKNOWN;
  }
}

char **CgiMaster::envToCArray()
{
  char **envp = NULL;
  try
  {
    envp = new char *[env_.size() + 1];
    int i = 0;
    for (std::map<std::string, std::string>::iterator it = env_.begin();
         it != env_.end(); ++it)
    {
      std::string env = it->first + "=" + it->second;
      envp[i] = new char[env.length() + 1];
      strcpy(envp[i], env.c_str());
      ++i;
    }
    envp[i] = NULL;
  }
  catch (std::bad_alloc &ba)
  {
    for (size_t j = 0; j < env_.size(); ++j)
    {
      delete[] envp[j];
    }
    delete[] envp;
    throw SysCallFailed("new");
  }
  return envp;
}

std::pair<pid_t, int> CgiMaster::execute()
{
  // Check if CGI path is legit
  struct stat buffer;
  if (stat(cgiPath_.c_str(), &buffer) != 0)
  {
    return std::make_pair(-1, errno); // Return errno for file not found or other stat errors
  }
  if (!(buffer.st_mode & S_IXUSR))
  {
    return std::make_pair(-1, EACCES); // Return EACCES for permission denied
  }

  pid_t pid_ = fork();
  if (pid_ == -1)
  {
    throw SysCallFailed("fork");
  }

  if (pid_ == 0)
  {
    handleChildProcess();
  }
  else
  {
    handleParentProcess();
  }
  return std::make_pair(pid_, outpipe_[0]);
}

void CgiMaster::handleChildProcess()
{
  close(inpipe_[1]);
  close(outpipe_[0]);
  dup2(inpipe_[0], STDIN_FILENO);
  dup2(outpipe_[1], STDOUT_FILENO);
  close(inpipe_[0]);
  close(outpipe_[1]);
  char cwd[PATH_MAX];
  std::string fullCgiPath;

  try {
    if (cgiPath_[0] != '/')
    {
      if (getcwd(cwd, sizeof(cwd)) == NULL)
        throw SysCallFailed("getcwd");
      fullCgiPath = std::string(cwd) + "/" + cgiPath_;
    }
    else
      fullCgiPath = cgiPath_;

    std::string cgiDir = fullCgiPath.substr(0, fullCgiPath.find_last_of('/'));
    if (chdir(cgiDir.c_str()) == -1)
    {
      throw SysCallFailed("chdir");
    }

    // envvar
    char **envp = envToCArray();

    // Use the pre-identified interpreter
    const char *interpreterPath = NULL;
    if (interpreter_ == PYTHON)
    {
      interpreterPath = "/usr/bin/python3";
    }
    else
    {
      interpreterPath = "/bin/sh";
    }

    char *argv[] = {const_cast<char *>(interpreterPath), const_cast<char *>(fullCgiPath.c_str()), NULL};
    // write(STDOUT_FILENO, "\n", 1);
    execve(interpreterPath, argv, envp);
    throw SysCallFailed("execve");
  }
  catch (SysCallFailed &e) {
    exit(EXIT_FAILURE);
  }
}

void CgiMaster::handleParentProcess()
{
  close(inpipe_[0]);
  close(outpipe_[1]);
  std::string body = request_->getBody();
  write(inpipe_[1], body.c_str(), body.length());
  close(inpipe_[1]);
}

static std::string getTime() {
  time_t rawtime;
  struct tm *timeinfo;
  char buffer[80];

  time(&rawtime);
  timeinfo = gmtime(&rawtime);

  strftime(buffer, sizeof(buffer), "Date: %a, %d %b %Y %H:%M:%S GMT", timeinfo);
  return std::string(buffer);
}

HttpResponse CgiMaster::convertCgiResponse(const std::string &cgiResponse)
{
  HttpResponse response;
  // std::istringstream iss(cgiResponse.substr(1)); // Skip first newline
  std::istringstream iss(cgiResponse);
  std::string line;
  bool statusSet = false;
  bool contentTypeSet = false;

  // Parse headers
  while (std::getline(iss, line) && !line.empty() && line != "\r\n")
  {
    size_t pos = line.find(":");
    if (pos != std::string::npos)
    {
      std::string key = line.substr(0, pos);
      std::string value = line.substr(pos + 1);
      if (key == "Status")
      {
        std::istringstream statusStream(value);
        std::string statusCode;
        statusStream >> statusCode;
        const unsigned int range[] = {100, 599};
        try {
          response.setStatus(static_cast<HttpStatus>(ft::stoui(statusCode, range)));
        } catch (ArgOutOfRange &e) {
          response.setStatus(BAD_GATEWAY);
          return response;
        }
        if (!(statusStream >> statusCode) || statusCode != http::statusToString(response.getStatus()))
        {
          response.setStatus(BAD_GATEWAY);
          return response;
        }
        statusSet = true;
      }
      else
      {
        response.setHeader(key, value);
        if (key == "Content-Type")
        {
          contentTypeSet = true;
        }
      }
    }
    else
    {
      response.setStatus(BAD_GATEWAY); // Malformed header
      return response;
    }
  }

  // Ensure Content-Type header is present
  if (!contentTypeSet)
  {
    response.setStatus(BAD_GATEWAY);
    return response;
  }

  // Set status to 200 OK if not included in CGI response
  if (!statusSet)
  {
    response.setStatus(OK);
  }

  // Add Date header if status is not 204 or 304
  HttpStatus status = response.getStatus();
  if (status != NO_CONTENT && status != NOT_MODIFIED)
  {
    response.setHeader("Date", getTime());
  }

  // Parse body
  std::string body;
  while (std::getline(iss, line))
  {
    if (!line.empty())
    {
      body += line + "\n";
    }
  }

  // Ensure body is not empty if expected
  if (body.empty() && status != NO_CONTENT && status != NOT_MODIFIED)
  {
    response.setStatus(BAD_GATEWAY);
    return response;
  }

  response.setBody(body);

  // Set Content-Length header
  response.setHeader("Content-Length", ft::uitost(body.length()));

  return response;
}
