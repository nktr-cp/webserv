#include "http_request.hpp"

const char *HttpRequest::parseMethod(const char *req) {
  this->method_ = kMethodTrie.search(req, ' ');
  size_t len = http::methodToString(this->method_).size();
  if (this->method_ == NONE || req[len] != ' ') {
    throw http::responseStatusException(NOT_IMPLEMENTED);
  }
  return req + len + 1;
}

const char *HttpRequest::parseUri(const char *req) {
  this->contentLength_ = 0;
  // Parse URI
  std::size_t i = 0;
  for (; req[i] != ' ' && req[i] != '?'; i++) {
    if (++this->contentLength_ >= kMaxUriSize) {
      throw http::responseStatusException(URI_TOO_LONG);
    }
    if (req[i] < '!' || req[i] > '~') {
      throw http::responseStatusException(BAD_REQUEST);
    }
  }
  this->uri_ = std::string(req, i);
  if (this->uri_.empty() || (req[i] != ' ' && req[i] != '?')) {
    throw http::responseStatusException(BAD_REQUEST);
  }
  req += i;
  // Parse query
  if (*req == '?') {
    req++;
    if (++this->contentLength_ >= kMaxUriSize) {
      throw http::responseStatusException(URI_TOO_LONG);
    }
    while (true) {
      // Parse key
      std::string key;
      i = 0;
      for (; req[i] && (req[i] != '&' && req[i] != '=' && req[i] != ' '); i++) {
        if (++this->contentLength_ >= kMaxUriSize) {
          throw http::responseStatusException(URI_TOO_LONG);
        }
        if (req[i] < '!' || req[i] > '~') {
          throw http::responseStatusException(BAD_REQUEST);
        }
      }
      key = std::string(req, i);
      // Parse value
      if (req[i] == '=') {
        req += i + 1;
        i = 0;
        for (; req[i] && (req[i] != '&' && req[i] != ' '); i++) {
          if (++this->contentLength_ >= kMaxUriSize) {
            throw http::responseStatusException(URI_TOO_LONG);
          }
          if (req[i] < '!' || req[i] > '~') {
            throw http::responseStatusException(BAD_REQUEST);
          }
        }
        this->query_[key] = std::string(req, i);
      } else {
        this->query_[key] = "";
      }
      req += i;
      if (*req == '&') {
        if (++this->contentLength_ >= kMaxUriSize) {
          throw http::responseStatusException(URI_TOO_LONG);
        }
        req++;
      } else {
        break;
      }
    }
  }
  if (*req != ' ') {
    throw http::responseStatusException(BAD_REQUEST);
  }
  return req + 1;
}

const char *HttpRequest::parseVersion(const char *req) {
  // Parse version
  std::size_t i = 0;
  for (; req[i] && req[i] != '\r'; i++) {
    if (req[i] < '!' || req[i] > '~') {
      throw http::responseStatusException(BAD_REQUEST);
    }
  }
  this->version_ = std::string(req, i);
  if (this->version_.empty() || req[i] != '\r' || req[i + 1] != '\n' ||
      this->version_ != VersionInfo::kHttpVersion) {
    throw http::responseStatusException(BAD_REQUEST);
  }
  return req + i + 2;
}

const char *HttpRequest::parseHeader(const char *req) {
  this->contentLength_ = 0;
  while (*req && req[0] != '\r') {
    size_t i = 0;
    for (; req[i] && req[i] != ':'; i++) {
      if (++this->contentLength_ >= kMaxHeaderSize) {
        throw http::responseStatusException(REQUEST_HEADER_FIELDS_TOO_LARGE);
      }
    }
    if (req[i] != ':' || req[i + 1] != ' ') {
      throw http::responseStatusException(BAD_REQUEST);
    }
    std::string key = std::string(req, i);
    i += 2;  // Skip ": "
    this->contentLength_ += 2;
    if (contentLength_ >= kMaxHeaderSize) {
      throw http::responseStatusException(REQUEST_HEADER_FIELDS_TOO_LARGE);
    }
    req += i;
    i = 0;
    for (; req[i] && req[i] != '\r'; i++) {
      if (++this->contentLength_ >= kMaxHeaderSize) {
        throw http::responseStatusException(REQUEST_HEADER_FIELDS_TOO_LARGE);
      }
    }
    if (req[i] != '\r' || req[i + 1] != '\n') {
      throw http::responseStatusException(BAD_REQUEST);
    }
    if (i == 0) {
      this->headers_[key] = "";
    } else {
      this->headers_[key] = std::string(req, i);
    }
    req += i + 2;  // Skip "\r\n"
    this->contentLength_ += 2;
    if (contentLength_ >= kMaxHeaderSize) {
      throw http::responseStatusException(REQUEST_HEADER_FIELDS_TOO_LARGE);
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
          throw http::responseStatusException(BAD_REQUEST);
        }
      }
      this->hostName_ = std::string(host, 0, i);
      if (i == host.size()) {
        this->hostPort_ = "80";
      } else {
        this->hostPort_ = std::string(host, i + 1);
      }
    } catch (std::out_of_range &e) {
      throw http::responseStatusException(BAD_REQUEST);
    } catch (http::responseStatusException &e) {
      throw e;
    }
  } catch (http::responseStatusException &e) {
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
      throw http::responseStatusException(PAYLOAD_TOO_LARGE);
    } catch (std::bad_alloc &e) {
      throw http::responseStatusException(PAYLOAD_TOO_LARGE);
    }
    if (this->body_.size() >= kMaxPayloadSize) {
      throw http::responseStatusException(PAYLOAD_TOO_LARGE);
    }
  } else {
    throw http::responseStatusException(BAD_REQUEST);
  }
}
HttpRequest::HttpRequest(const HttpRequest &src) { *this = src; }
HttpRequest &HttpRequest::operator=(const HttpRequest &src) {
  if (this != &src) {
    this->method_ = src.method_;
    this->uri_ = src.uri_;
    this->query_ = src.query_;
    this->hostName_ = src.hostName_;
    this->hostPort_ = src.hostPort_;
    this->version_ = src.version_;
    this->headers_ = src.headers_;
    this->body_ = src.body_;
    this->contentLength_ = src.contentLength_;
  }
  return *this;
}
HttpRequest::~HttpRequest() {}

TrieNode<HttpMethod> initialize_method_trie() {
  TrieNode<HttpMethod> root;
  root.insert(http::methodToString(GET).c_str(), GET);
  root.insert(http::methodToString(HEAD).c_str(), HEAD);
  root.insert(http::methodToString(POST).c_str(), POST);
  root.insert(http::methodToString(PUT).c_str(), PUT);
  root.insert(http::methodToString(DELETE).c_str(), DELETE);
  root.insert(http::methodToString(CONNECT).c_str(), CONNECT);
  root.insert(http::methodToString(OPTIONS).c_str(), OPTIONS);
  root.insert(http::methodToString(TRACE).c_str(), TRACE);
  root.insert(http::methodToString(PATCH).c_str(), PATCH);
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
const std::string &HttpRequest::getQueryAsStr() const {
  static std::string query;
  query.clear();
  for (dict::const_iterator it = this->query_.begin(); it != this->query_.end();
       it++) {
    query += it->first + "=" + it->second + "&";
  }
  if (!query.empty()) {
    query.erase(query.end() - 1);
  }
  return query;
}
const std::string &HttpRequest::getHostName() const { return this->hostName_; }
const std::string &HttpRequest::getHostPort() const { return this->hostPort_; }
const std::string &HttpRequest::getVersion() const { return this->version_; }
const dict &HttpRequest::getHeader() const { return this->headers_; }
const std::string &HttpRequest::getHeader(const std::string &key) const {
  dict::const_iterator it = this->headers_.find(key);
  if (it != this->headers_.end()) {
    return it->second;
  } else {
    static const std::string e;
    return e;
  }
}
const std::string &HttpRequest::getBody() const { return this->body_; }
