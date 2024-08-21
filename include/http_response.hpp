#pragma once
#include <map>
#include <string>

#include "typedefs.hpp"
#include "utils.hpp"

std::string to_string(HttpStatus status);

// statusはデフォルトでOK
class HttpResponse {
 private:
  static const std::string kHttpVersion;
  std::string status_;
  dict headers_;
  std::string body_;

 public:
  HttpResponse();
  HttpResponse(HttpStatus status);
  HttpResponse(const HttpResponse &src);
  HttpResponse &operator=(const HttpResponse &src);
  ~HttpResponse();

  void setStatus(HttpStatus status);
  void setHeader(const std::string &key, const std::string &value);
  void setHeader(const dict &headers);
  void setBody(const std::string &body);

  // content-lengthヘッダーを追加して、エンコードしたレスポンスを返す
  std::string encode() const;
};