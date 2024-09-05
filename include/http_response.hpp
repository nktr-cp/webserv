#pragma once
#include <map>
#include <string>
#include <vector>
#include <sstream>

#include "typedefs.hpp"
#include "utils.hpp"

namespace http {
  std::string statusToString(HttpStatus status);
  class responseStatusException : public std::exception {
   private:
    HttpStatus status_;

   public:
    responseStatusException(HttpStatus status);
    HttpStatus getStatus() const;
    const char *what() const throw();
  };
} // namespace http

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