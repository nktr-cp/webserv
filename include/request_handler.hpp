#pragma once

#include "http_request.hpp"
#include "http_response.hpp"
#include "location.hpp"
#include "config.hpp"

class RequestHandler {
private:
  HttpRequest *request_;
  HttpResponse *response_;
  Location *location_; // 静的リクエストでは使用しないはずなので、未実装
  std::string rootPath_;
  std::string relativePath_;

  void handleStaticGet();
  void handleStaticPost();
  void handleStaticDelete();

public:
  RequestHandler();
  RequestHandler(HttpRequest &request, HttpResponse &response, ServerConfig &config);
  RequestHandler(const RequestHandler &src);
  ~RequestHandler();
  RequestHandler &operator=(const RequestHandler &src);

  void process();
};
