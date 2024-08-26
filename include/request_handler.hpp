#pragma once

#include <dirent.h>

#include <fstream>
#include <iomanip>
#include <sstream>

#include "config.hpp"
#include "http_request.hpp"
#include "http_response.hpp"
#include "location.hpp"
#include "typedefs.hpp"
#include "utils.hpp"

class RequestHandler {
 private:
  HttpRequest   *request_;
  HttpResponse  *response_;
  Location      location_;  // 静的リクエストでは使用しないはずなので、未実装
  std::string   rootPath_;
  std::string   relativePath_;

  std::string getMimeType(const std::string &path);
  std::string generateDirectoryListing(const std::string &path);

  void handleStaticGet();
  void handleStaticPost();
  void handleStaticDelete();

 public:
  RequestHandler();
  RequestHandler(HttpRequest &request, HttpResponse &response,
                 ServerConfig &config);
  RequestHandler(const RequestHandler &src);
  ~RequestHandler();
  RequestHandler &operator=(const RequestHandler &src);

  void process();
};
