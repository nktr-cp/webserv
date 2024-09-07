#include "http_request.hpp"

#include <map>
#include <stdexcept>
#include <string>
#include <climits>

#include "server.hpp"
#include "trie_node.hpp"
#include "typedefs.hpp"

std::string to_string(HttpMethod method) {
  switch (method) {
    case GET:
      return "GET";
    case POST:
      return "POST";
    case DELETE:
      return "DELETE";
    default:
      return "NONE";
  }
}

const char *HttpRequest::parseMethod(const char *req) {
  this->method_ = kMethodTrie.search(req, ' ');
  size_t len = to_string(this->method_).size();
  if (this->method_ == NONE || req[len] != ' ') {
    throw BadRequestException();
  }
  return req + len + 1;
}

#include <iostream>
const char *HttpRequest::parseUri(const char *req) {
  this->contentLength_ = 0;
  // Parse URI
  std::size_t i = 0;
  for (; req[i] != ' ' && req[i] != '?'; i++) {
    if (++this->contentLength_ >= kMaxUriSize) {
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
    if (++this->contentLength_ >= kMaxUriSize) {
      throw UriTooLongException();
    }
    while (true) {
      // Parse key
      std::string key;
      i = 0;
      for (; req[i] && (req[i] != '&' && req[i] != '=' && req[i] != ' '); i++) {
        if (++this->contentLength_ >= kMaxUriSize) {
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
          if (++this->contentLength_ >= kMaxUriSize) {
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
        if (++this->contentLength_ >= kMaxUriSize) {
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

const char *HttpRequest::parseVersion(const char *req) {
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

const char *HttpRequest::parseHeader(const char *req) {
  this->contentLength_ = 0;
  while (*req && req[0] != '\r') {
    size_t i = 0;
    for (; req[i] && req[i] != ':'; i++) {
      if (++this->contentLength_ >= kMaxHeaderSize) {
        throw RequestHeaderFieldsTooLargeException();
      }
    }
    if (req[i] != ':' || req[i + 1] != ' ') {
      throw BadRequestException();
    }
    std::string key = std::string(req, i);
    i += 2;  // Skip ": "
    this->contentLength_ += 2;
    if (contentLength_ >= kMaxHeaderSize) {
      throw RequestHeaderFieldsTooLargeException();
    }
    req += i;
    i = 0;
    for (; req[i] && req[i] != '\r'; i++) {
      if (++this->contentLength_ >= kMaxHeaderSize) {
        throw RequestHeaderFieldsTooLargeException();
      }
    }
    if (req[i] != '\r' || req[i + 1] != '\n') {
      throw BadRequestException();
    }
    if (i == 0) {
      this->headers_[key] = "";
    } else {
      this->headers_[key] = std::string(req, i);
    }
    req += i + 2;  // Skip "\r\n"
    this->contentLength_ += 2;
    if (contentLength_ >= kMaxHeaderSize) {
      throw RequestHeaderFieldsTooLargeException();
    }
  }
  return req;
}

HttpRequest::HttpRequest() {}
HttpRequest::HttpRequest(const char *raw_request) {
  try {
    raw_request = this->parseMethod(raw_request);
    raw_request = this->parseUri(raw_request);
    raw_request = this->parseVersion(raw_request);
    raw_request = this->parseHeader(raw_request);
    try {
      // Parse Host header
      std::string host = this->headers_.at("Host");
      size_t i = 0;
      for (; i < host.size() && host[i] != ':'; i++) {
        if (!std::isalnum(host[i]) && host[i] != '.' && host[i] != '-') {
          throw BadRequestException();
        }
      }
      this->hostName_ = std::string(host, 0, i);
      if (i == host.size()) {
        this->hostPort_ = "80";
      } else {
        this->hostPort_ = std::string(host, i + 1);
      }
    } catch (std::out_of_range &e) {
      throw BadRequestException();
    } catch (RequestException &e) {
      throw e;
    }
  } catch (RequestException &e) {
    throw e;
  }
  this->contentLength_ = 0;
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
const size_t HttpRequest::kMaxHeaderSize = 8192;
const size_t HttpRequest::kMaxPayloadSize = INT_MAX;
const size_t HttpRequest::kMaxUriSize = 1024;

HttpMethod HttpRequest::getMethod() const { return this->method_; }
const std::string &HttpRequest::getUri() const { return this->uri_; }
const dict &HttpRequest::getQuery() const { return this->query_; }
const std::string &HttpRequest::getQuery(const std::string &key) const {
  return this->query_.at(key);
}
const std::string &HttpRequest::getHostName() const {
  return this->hostName_;
}
const std::string &HttpRequest::getHostPort() const {
  return this->hostPort_;
}
const std::string &HttpRequest::getVersion() const { return this->version_; }
const dict &HttpRequest::getHeader() const { return this->headers_; }
const std::string &HttpRequest::getHeader(const std::string &key) const {
  return this->headers_.at(key);
}
const std::string &HttpRequest::getBody() const { return this->body_; }

HttpRequest::RequestException::RequestException(HttpStatus status)
    : httpStatus_(status), message_(NULL) {}
HttpRequest::RequestException::RequestException(HttpStatus status,
                                                const char *message)
    : httpStatus_(status), message_(message) {}
const char *HttpRequest::RequestException::what() const throw() {
  return this->message_;
}
HttpStatus HttpRequest::RequestException::getStatus() const {
  return this->httpStatus_;
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