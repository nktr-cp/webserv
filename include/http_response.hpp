#pragma once
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "typedefs.hpp"
#include "utils.hpp"

// デフォルトのstatusはOK
class HttpResponse {
 private:
  HttpStatus status_;
  dict headers_;
  std::string body_;

 public:
  HttpResponse();
  HttpResponse(HttpStatus status);
  HttpResponse(const HttpResponse &src);
  HttpResponse &operator=(const HttpResponse &src);
  ~HttpResponse();

  HttpStatus getStatus() const;
  void setStatus(HttpStatus status);
  void setHeader(const std::string &key, const std::string &value);
  void setHeader(const dict &headers);
  void setBody(const std::string &body);

  // content-lengthヘッダーを追加して、エンコードしたレスポンスを返す
  std::string encode() const;
};