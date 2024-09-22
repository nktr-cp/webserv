#include "http_response.hpp"

HttpResponse::HttpResponse() : status_(OK), headers_(dict()), body_("") {
  setHeader("Content-Type", "text/html");
}

HttpResponse::HttpResponse(HttpStatus status)
    : status_(status), headers_(dict()), body_("") {
  setHeader("Content-Type", "text/html");
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

HttpStatus HttpResponse::getStatus() const { return this->status_; }

void HttpResponse::setStatus(HttpStatus status) {
  this->status_ = status;
  if (body_.empty()) {
    std::stringstream ss;
    ss << "<html>";
    ss << "<head><title>" << status << " " << http::statusToString(status)
       << "</title></head>";
    ss << "<body>";
    ss << "<center><h1>" << status << " " << http::statusToString(status)
       << "</h1></center>";
    ss << "<hr><center>" << VersionInfo::kProgramName << "/"
       << VersionInfo::kProgramVersion << "</center>";
    ss << "</body>";
    ss << "</html>";
    ss << "<!--";
    for (int i = ss.str().length(); i < FRIENDLY_ERROR_PAGE_LENGTH;) {
      ss << VersionInfo::kProgramName;
      i += VersionInfo::kProgramName.length();
    }
    ss << "-->";
    this->body_ = ss.str();
  }
}

void HttpResponse::setHeader(const std::string &key, const std::string &value) {
  this->headers_[key] = value;
}

void HttpResponse::setHeader(const dict &headers) { this->headers_ = headers; }

void HttpResponse::setBody(const std::string &body) { this->body_ = body; }

std::string HttpResponse::encode() const {
  std::ostringstream oss;
  oss << VersionInfo::kHttpVersion << " " << this->status_ << " "
      << http::statusToString(this->status_) << "\r\n";
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