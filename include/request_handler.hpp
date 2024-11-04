#pragma once

#include <dirent.h>

#include <fstream>
#include <iomanip>
#include <sstream>

#include "cgi.hpp"
#include "config.hpp"
#include "http_request.hpp"
#include "http_response.hpp"
#include "location.hpp"
#include "typedefs.hpp"
#include "utils.hpp"

struct FileEntry
{
  std::string name;
  std::string modTime;
  long size;
  bool isDirectory;

  FileEntry(const std::string &n, const std::string &m, long s, bool d);
};

class RequestHandler
{
private:
  HttpRequest *request_;
  HttpResponse *response_;
  const ServerConfig *config_;
  const Location *location_;
  std::string rootPath_;
  std::string relativePath_;
  // CGI

  std::string getMimeType(const std::string &path);
  std::string generateDirectoryListing(const std::string &path);

  void handleStaticGet();
  void handleStaticPost();
  void handleStaticDelete();
  void handleCGIRequest();

public:
  RequestHandler();
  RequestHandler(HttpRequest &request, HttpResponse &response,
                 ServerConfig &config);
  RequestHandler(const RequestHandler &src);
  ~RequestHandler();
  RequestHandler &operator=(const RequestHandler &src);

  void process();
};
