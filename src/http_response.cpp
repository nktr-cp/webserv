#include "http_response.hpp"

namespace Http {
std::string statusToString(HttpStatus status) {
  switch (status) {
    case OK:
      return "200 OK";
    case BAD_REQUEST:
      return "400 Bad Request";
    case FORBIDDEN:
      return "403 Forbidden";
    case NOT_FOUND:
      return "404 Not Found";
    case METHOD_NOT_ALLOWED:
      return "405 Method Not Allowed";
    case REQUEST_TIMEOUT:
      return "408 Request Timeout";
    case PAYLOAD_TOO_LARGE:
      return "413 Payload Too Large";
    case URI_TOO_LONG:
      return "414 URI Too Long";
    case REQUEST_HEADER_FIELDS_TOO_LARGE:
      return "431 Request Header Fields Too Large";
    case INTERNAL_SERVER_ERROR:
      return "500 Internal Server Error";
    default:
      return "500 Internal Server Error";
  }
}
}  // namespace Http

const std::string HttpResponse::kHttpVersion = "HTTP/1.1";

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
    this->body_ = "<h1>" + Http::statusToString(status) + "</h1>";
  }
}

void HttpResponse::setHeader(const std::string &key, const std::string &value) {
  this->headers_[key] = value;
}

void HttpResponse::setHeader(const dict &headers) { this->headers_ = headers; }

void HttpResponse::setBody(const std::string &body) { this->body_ = body; }

std::string HttpResponse::encode() const {
  std::ostringstream oss;
  oss << this->kHttpVersion << " " << Http::statusToString(this->status_)
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
