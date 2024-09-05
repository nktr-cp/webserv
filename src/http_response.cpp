#include "http_response.hpp"

namespace http {
std::string statusToString(HttpStatus status) {
  switch (status) {
    case OK:
      return "OK";
    case FOUND:
      return "Found";
    case TEMPORARY_REDIRECT:
      return "Temporary Redirect";
    case PERMANENT_REDIRECT:
      return "Permanent Redirect";
    case BAD_REQUEST:
      return "Bad Request";
    case UNAUTHORIZED:
      return "Unauthorized";
    case FORBIDDEN:
      return "Forbidden";
    case NOT_FOUND:
      return "Not Found";
    case METHOD_NOT_ALLOWED:
      return "Method Not Allowed";
    case LENGTH_REQUIRED:
      return "Length Required";
    case PAYLOAD_TOO_LARGE:
      return "Payload Too Large";
    case URI_TOO_LONG:
      return "URI Too Long";
    case TOO_MANY_REQUESTS:
      return "Too Many Requests";
    case INTERNAL_SERVER_ERROR:
      return "Internal Server Error";
    case NOT_IMPLEMENTED:
      return "Not Implemented";
    case BAD_GATEWAY:
      return "Bad Gateway";
    case SERVICE_UNAVAILABLE:
      return "Service Unavailable";
    case GATEWAY_TIMEOUT:
      return "Gateway Timeout";
    default:
      return "I'm a teapot";
  }
}

responseStatusException::responseStatusException(HttpStatus status)
    : status_(status) {}

const char *responseStatusException::what() const throw() {
  return (std::to_string(this->status_) + " " + statusToString(this->status_))
      .c_str();
}

HttpStatus responseStatusException::getStatus() const {
  return this->status_;
}
}  // namespace http

HttpResponse::HttpResponse() : status_(OK), headers_(dict()), body_("") {}

HttpResponse::HttpResponse(HttpStatus status)
    : status_(status), headers_(dict()), body_("") {}

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

HttpStatus HttpResponse::getStatus() const { return this->status_; }

void HttpResponse::setStatus(HttpStatus status) {
  this->status_ = status;
  if (body_.empty()) {
    this->setHeader("Content-Type", "text/html");
    this->body_ = "<h1>" + std::to_string(status) + " " + http::statusToString(status) + "</h1>";
  }
}

void HttpResponse::setHeader(const std::string &key, const std::string &value) {
  this->headers_[key] = value;
}

void HttpResponse::setHeader(const dict &headers) { this->headers_ = headers; }

void HttpResponse::setBody(const std::string &body) { this->body_ = body; }

std::string HttpResponse::encode() const {
  std::ostringstream oss;
  oss << HTTP_VERSION << " " << this->status_ << " " << http::statusToString(this->status_)
      << "\r\n";
  for (std::map<std::string, std::string>::const_iterator it =
           this->headers_.begin();
       it != this->headers_.end(); ++it) {
    oss << it->first << ": " << it->second << "\r\n";
  }
  oss << "Content-Length: " << ft::uitost(this->body_.length()) << "\r\n";
  oss << "\r\n";
  oss << this->body_;
  return oss.str();
}
