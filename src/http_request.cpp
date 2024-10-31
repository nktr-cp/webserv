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
  size_t len = 0;
  // Parse URI
  std::size_t i = 0;
  for (; req[i] != ' ' && req[i] != '?'; i++) {
    if (++len >= kMaxUriSize) {
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
    if (++len >= kMaxUriSize) {
      throw http::responseStatusException(URI_TOO_LONG);
    }
    while (true) {
      // Parse key
      std::string key;
      i = 0;
      for (; req[i] && (req[i] != '&' && req[i] != '=' && req[i] != ' '); i++) {
        if (++len >= kMaxUriSize) {
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
          if (++len >= kMaxUriSize) {
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
        if (++len >= kMaxUriSize) {
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
  size_t len = 0;
  while (*req && req[0] != '\r') {
    size_t i = 0;
    for (; req[i] && req[i] != ':'; i++) {
      if (++len >= kMaxHeaderSize) {
        throw http::responseStatusException(REQUEST_HEADER_FIELDS_TOO_LARGE);
      }
    }
    if (req[i] != ':' || req[i + 1] != ' ') {
      throw http::responseStatusException(BAD_REQUEST);
    }
    std::string key = std::string(req, i);
    i += 2;  // Skip ": "
    len += 2;
    if (len >= kMaxHeaderSize) {
      throw http::responseStatusException(REQUEST_HEADER_FIELDS_TOO_LARGE);
    }
    req += i;
    i = 0;
    for (; req[i] && req[i] != '\r'; i++) {
      if (++len >= kMaxHeaderSize) {
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
    len += 2;
    if (len >= kMaxHeaderSize) {
      throw http::responseStatusException(REQUEST_HEADER_FIELDS_TOO_LARGE);
    }
  }
  // validation
  if (this->headers_.find("Host") == this->headers_.end()) {
    throw http::responseStatusException(BAD_REQUEST);
  } else {
    std::string host = this->headers_["Host"];
    size_t i = 0;
    while (i < host.size() && host[i] != ':') {
      i++;
    }
    if (i == 0) {
      throw http::responseStatusException(BAD_REQUEST);
    }
    this->hostName_ = host.substr(0, i);
    if (i == host.size()) {
      this->hostPort_ = DEFAULT_PORT;
    } else {
      this->hostPort_ = host.substr(i + 1);
    }
  }
  bool hasContentLength = headers_.find("Content-Length") != headers_.end();
  bool hasTransferEncoding = headers_.find("Transfer-Encoding") != headers_.end();
  if (hasContentLength && hasTransferEncoding) {
    throw http::responseStatusException(BAD_REQUEST);
  } else if (!hasContentLength && !hasTransferEncoding) {
    if (method_ == POST || method_ == PUT) {
      throw http::responseStatusException(LENGTH_REQUIRED);
    }
  }
  // set content length
  if (hasContentLength) {
    std::stringstream ss(headers_["Content-Length"]);
    ss >> contentLength_;
    if (ss.fail() || contentLength_ < 0 || static_cast<size_t>(contentLength_) > kMaxPayloadSize) {
      throw http::responseStatusException(BAD_REQUEST);
    }
  }
  else if (hasTransferEncoding) {
    contentLength_ = -1;
  } else {
    contentLength_ = 0;
  }
  return req;
}

HttpRequest::HttpRequest()
  : buffer_(), method_(NONE), uri_(), query_(), hostName_(), hostPort_(),
    version_(), headers_(), body_(), contentLength_(0), progress(HEADER)
  {}

static size_t is_end_of_header(const std::string payload) {
  size_t i = 0;
  while (payload[i]) {
    if (payload[i] == '\r' && payload[i + 1] == '\n' &&
        payload[i + 2] == '\r' && payload[i + 3] == '\n') {
      return i + 4;
    }
    i++;
  }
  return 0;
}
void HttpRequest::parseRequest(const char *payload) {
  try {
    buffer_ += payload;
    switch (progress) {
      case HEADER:
        goto flag_header;
      case BODY:
        goto flag_body;
      case DONE:
        throw http::responseStatusException(BAD_REQUEST);
    }
    flag_header: {
      size_t endOfHeader = is_end_of_header(buffer_);
      if (!endOfHeader) {
        return;
      } else {
        try {
          const char* cur = buffer_.c_str();
          cur = this->parseMethod(cur);
          cur = this->parseUri(cur);
          cur = this->parseVersion(cur);
          cur = this->parseHeader(cur);
          progress = BODY;
          buffer_ = buffer_.substr(endOfHeader);
        } catch (http::responseStatusException &e) {
          throw e;
        } catch (std::exception &e) {
          std::cerr << e.what() << std::endl;
          throw http::responseStatusException(INTERNAL_SERVER_ERROR);
        }
      }
    }
    flag_body: {
      std::cerr << "contentLength: " << contentLength_ << std::endl;
      if (contentLength_ >= 0) {
        if (buffer_.size() < static_cast<size_t>(contentLength_)) {
          return;
        } else {
          body_ = buffer_.substr(0, contentLength_);
          progress = DONE;
          return;
        }
        return;
      } else {
        // unchunk
        std::string::const_iterator it = buffer_.begin();
        while (it != buffer_.end()) {
          size_t chunkSize = 0;
          while (it != buffer_.end() && *it != '\r') {
            if (*it >= '0' && *it <= '9') {
              chunkSize = chunkSize * 16 + *it - '0';
            } else if (*it >= 'a' && *it <= 'f') {
              chunkSize = chunkSize * 16 + *it - 'a' + 10;
            } else if (*it >= 'A' && *it <= 'F') {
              chunkSize = chunkSize * 16 + *it - 'A' + 10;
            } else {
              throw http::responseStatusException(BAD_REQUEST);
            }
            it++;
          }
          if (*it != '\r' || *(it + 1) != '\n') {
            throw http::responseStatusException(BAD_REQUEST);
          }
          if (chunkSize == 0) {
            progress = DONE;
            return;
          }
          it += 2;
          std::string chunk;
          for (size_t i=0; i<chunkSize; i++) {
            if (it == buffer_.end()) {
              return;
            }
            chunk += *it;
            it++;
          }
          if (it == buffer_.end() || *it != '\r' || *(it + 1) != '\n') {
            throw http::responseStatusException(BAD_REQUEST);
          }
          body_ += chunk;
          buffer_ += it - buffer_.begin() + 2;
        }
      }
    }
  } catch (http::responseStatusException &e) {
    throw e;
  } catch (std::exception &e) {
    throw http::responseStatusException(INTERNAL_SERVER_ERROR);
  }
}
HttpRequest::HttpRequest(const HttpRequest &src) { *this = src; }
HttpRequest &HttpRequest::operator=(const HttpRequest &src) {
  if (this != &src) {
    this->buffer_ = src.buffer_;
    this->method_ = src.method_;
    this->uri_ = src.uri_;
    this->query_ = src.query_;
    this->hostName_ = src.hostName_;
    this->hostPort_ = src.hostPort_;
    this->version_ = src.version_;
    this->headers_ = src.headers_;
    this->body_ = src.body_;
    this->contentLength_ = src.contentLength_;
    this->progress = src.progress;
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
const size_t HttpRequest::kMaxPayloadSize = 16384;
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
