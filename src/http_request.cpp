#include "http_request.hpp"

#include <map>
#include <stdexcept>
#include <string>

#include "trie_node.hpp"

std::string to_string(HttpMethod method) {
  switch (method) {
    case GET:
      return "GET";
    case HEAD:
      return "HEAD";
    case POST:
      return "POST";
    case OPTIONS:
      return "OPTIONS";
    case PUT:
      return "PUT";
    case DELETE:
      return "DELETE";
    default:
      return "NONE";
  }
}

const char *HttpRequest::parse_method(const char *req) {
  this->method_ = kMethodTrie.search(req, ' ');
  size_t len = to_string(this->method_).size();
  if (this->method_ == NONE || req[len] != ' ') {
    throw BadRequestException();
  }
  return req + len + 1;
}

#include <iostream>
const char *HttpRequest::parse_uri(const char *req) {
  this->content_length_ = 0;
  // Parse URI
  std::size_t i = 0;
  for (; req[i] != ' ' && req[i] != '?'; i++) {
    if (++this->content_length_ >= kMaxUriSize) {
      throw UriTooLongException();
    }
    if (req[i] < '!' || req[i] > '~') {
      throw BadRequestException();
    }
  }
  this->uri_ = std::string(req, i);
  if (this->uri_.empty() || (req[i] != ' ' && req[i] != '?')) {
    throw BadRequestException();
  }
  req += i;
  // Parse query
  if (*req == '?') {
    req++;
    if (++this->content_length_ >= kMaxUriSize) {
      throw UriTooLongException();
    }
    while (true) {
      // Parse key
      std::string key;
      i = 0;
      for (; req[i] && (req[i] != '&' && req[i] != '=' && req[i] != ' '); i++) {
        if (++this->content_length_ >= kMaxUriSize) {
          throw UriTooLongException();
        }
        if (req[i] < '!' || req[i] > '~') {
          throw BadRequestException();
        }
      }
      key = std::string(req, i);
      // Parse value
      if (req[i] == '=') {
        req += i + 1;
        i = 0;
        for (; req[i] && (req[i] != '&' && req[i] != ' '); i++) {
          if (++this->content_length_ >= kMaxUriSize) {
            throw UriTooLongException();
          }
          if (req[i] < '!' || req[i] > '~') {
            throw BadRequestException();
          }
        }
        this->query_[key] = std::string(req, i);
      } else {
        this->query_[key] = "";
      }
      req += i;
      if (*req == '&') {
        if (++this->content_length_ >= kMaxUriSize) {
          throw UriTooLongException();
        }
        req++;
      } else {
        break;
      }
    }
  }
  if (*req != ' ') {
    throw BadRequestException();
  }
  return req + 1;
}

const char *HttpRequest::parse_version(const char *req) {
  // Parse version
  std::size_t i = 0;
  for (; req[i] && req[i] != '\r'; i++) {
    if (req[i] < '!' || req[i] > '~') {
      throw BadRequestException();
    }
  }
  this->version_ = std::string(req, i);
  if (this->version_.empty() || req[i] != '\r' || req[i + 1] != '\n' ||
      this->version_ != "HTTP/1.1") {
    throw BadRequestException();
  }
  return req + i + 2;
}

const char *HttpRequest::parse_header(const char *req) {
  this->content_length_ = 0;
  while (*req && req[0] != '\r') {
    size_t i = 0;
    for (; req[i] && req[i] != ':'; i++) {
      if (++this->content_length_ >= kMaxHeaderSize) {
        throw RequestHeaderFieldsTooLargeException();
      }
    }
    if (!req[i] || req[i] != ':' || req[i + 1] != ' ') {
      throw BadRequestException();
    }
    std::string key = std::string(req, i);
    i += 2;  // Skip ": "
    this->content_length_ += 2;
    if (content_length_ >= kMaxHeaderSize) {
      throw RequestHeaderFieldsTooLargeException();
    }
    req += i;
    for (; req[i] && req[i] != '\r'; i++) {
      if (++this->content_length_ >= kMaxHeaderSize) {
        throw RequestHeaderFieldsTooLargeException();
      }
    }
    if (!req[i] && req[i] != '\r' || req[i + 1] != '\n') {
      throw BadRequestException();
    }
    if (i == 0) {
      this->headers_[key] = "";
    } else {
      this->headers_[key] = std::string(req, i);
    }
    req += i + 2;  // Skip "\r\n"
    this->content_length_ += 2;
    if (content_length_ >= kMaxHeaderSize) {
      throw RequestHeaderFieldsTooLargeException();
    }
  }
  return req;
}

HttpRequest::HttpRequest() {}
HttpRequest::HttpRequest(const char *raw_request) {
  try {
    raw_request = this->parse_method(raw_request);
    raw_request = this->parse_uri(raw_request);
    raw_request = this->parse_version(raw_request);
    raw_request = this->parse_header(raw_request);
    try {
      // Parse Host header
      std::string host = this->headers_.at("Host");
      size_t i = 0;
      for (; i < host.size() && host[i] != ':'; i++) {
        if (!std::isalnum(host[i]) && host[i] != '.' && host[i] != '-') {
          throw BadRequestException();
        }
      }
      this->host_name_ = std::string(host, 0, i);
      if (i == host.size()) {
        this->host_port_ = "80";
      } else {
        this->host_port_ = std::string(host, i + 1);
      }
    } catch (std::out_of_range &e) {
      throw BadRequestException();
    } catch (RequestException &e) {
      throw e;
    }
  } catch (RequestException &e) {
    throw e;
  }
  if (!*raw_request) {
    this->body_ = "";
  } else if (raw_request[0] == '\r' && raw_request[1] == '\n') {
    raw_request += 2;
    try {
      this->body_ = std::string(raw_request);
    } catch (std::length_error &e) {
      throw PayloadTooLargeException();
    } catch (std::bad_alloc &e) {
      throw PayloadTooLargeException();
    }
    if (this->body_.size() >= kMaxPayloadSize) {
      throw PayloadTooLargeException();
    }
  } else {
    throw BadRequestException();
  }
}
HttpRequest::HttpRequest(const HttpRequest &src) { *this = src; }
HttpRequest &HttpRequest::operator=(const HttpRequest &src) {
  if (this != &src) {
    this->method_ = src.method_;
    this->uri_ = src.uri_;
    this->version_ = src.version_;
    this->headers_ = src.headers_;
    this->body_ = src.body_;
  }
  return *this;
}
HttpRequest::~HttpRequest() {}

TrieNode<HttpMethod> initialize_method_trie() {
  TrieNode<HttpMethod> root;
  root.insert("GET", GET);
  root.insert("HEAD", HEAD);
  root.insert("POST", POST);
  root.insert("OPTIONS", OPTIONS);
  root.insert("PUT", PUT);
  root.insert("DELETE", DELETE);
  return root;
}
const TrieNode<HttpMethod> HttpRequest::kMethodTrie = initialize_method_trie();
const int HttpRequest::kMaxHeaderSize = 8192;
const int HttpRequest::kMaxPayloadSize = 8192;
const int HttpRequest::kMaxUriSize = 1024;

HttpMethod HttpRequest::get_method() const { return this->method_; }
const std::string &HttpRequest::get_uri() const { return this->uri_; }
const dict &HttpRequest::get_query() const {
  return this->query_;
}
const std::string &HttpRequest::get_query(const std::string &key) const {
  return this->query_.at(key);
}
const std::string &HttpRequest::get_host_name() const {
  return this->host_name_;
}
const std::string &HttpRequest::get_host_port() const {
  return this->host_port_;
}
const std::string &HttpRequest::get_version() const { return this->version_; }
const dict &HttpRequest::get_header() const {
  return this->headers_;
}
const std::string &HttpRequest::get_header(const std::string &key) const {
  return this->headers_.at(key);
}
const std::string &HttpRequest::get_body() const { return this->body_; }

HttpRequest::RequestException::RequestException(HttpStatus http_status)
    : http_status_(http_status), message_(NULL) {}
HttpRequest::RequestException::RequestException(
  HttpStatus http_status, const char *message) {
  this->http_status_ = http_status;
}
const char *HttpRequest::RequestException::what() const throw() {
  return this->message_;
}
const HttpStatus HttpRequest::RequestException::get_status() const {
  return this->http_status_;
}

HttpRequest::BadRequestException::BadRequestException()
    : RequestException(BAD_REQUEST, "Bad Request") {}
HttpRequest::UriTooLongException::UriTooLongException()
    : RequestException(URI_TOO_LONG, "URI too long") {}
HttpRequest::RequestHeaderFieldsTooLargeException::
    RequestHeaderFieldsTooLargeException()
    : RequestException(REQUEST_HEADER_FIELDS_TOO_LARGE,
                       "Request header fields too large") {}
HttpRequest::PayloadTooLargeException::PayloadTooLargeException()
    : RequestException(PAYLOAD_TOO_LARGE, "Payload too large") {}
HttpRequest::InternalServerErrorException::InternalServerErrorException()
    : RequestException(INTERNAL_SERVER_ERROR, "Internal Server Error") {}