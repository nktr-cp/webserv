#include "http_response.hpp"

namespace Http {
std::string statusToString(HttpStatus status) {
  switch (status) {
    case OK:
      return "200 OK";
    case BAD_REQUEST:
      return "400 Bad Request";
    case NOT_FOUND:
      return "404 Not Found";
    case INTERNAL_SERVER_ERROR:
      return "500 Internal Server Error";
    default:
      return "500 Internal Server Error";
  }
}
}  // namespace Http

const std::string HttpResponse::kHttpVersion = "HTTP/1.1";

HttpResponse::HttpResponse() : headers_(dict()), body_("") {
  this->setStatus(OK);
}

HttpResponse::HttpResponse(HttpStatus status) : headers_(dict()), body_("") {
  this->setStatus(status);
}

HttpResponse::HttpResponse(const HttpResponse &src) {
  this->status_ = src.status_;
  this->headers_ = src.headers_;
  this->body_ = src.body_;
}

HttpResponse &HttpResponse::operator=(const HttpResponse &src) {
  if (this == &src) {
    return *this;
  }
  this->status_ = src.status_;
  this->headers_ = src.headers_;
  this->body_ = src.body_;
  return *this;
}

HttpResponse::~HttpResponse() {}

void HttpResponse::setStatus(HttpStatus status) {
  this->status_ = Http::statusToString(status);
}

void HttpResponse::setHeader(const std::string &key, const std::string &value) {
  this->headers_[key] = value;
}

void HttpResponse::setHeader(const dict &headers) { this->headers_ = headers; }

void HttpResponse::setBody(const std::string &body) { this->body_ = body; }

std::string HttpResponse::encode() const {
  std::string response = this->kHttpVersion + " " + this->status_ + "\r\n";
  for (std::map<std::string, std::string>::const_iterator it =
           this->headers_.begin();
       it != this->headers_.end(); ++it) {
    response += it->first + ": " + it->second + "\r\n";
  }
  if (this->body_ != "") {
    response += "Content-Length: " + ft::uitost(this->body_.length()) + "\r\n";
    response += "\r\n";
    response += this->body_;
  }
  return response;
}
