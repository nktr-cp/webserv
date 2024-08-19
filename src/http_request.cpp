#include "http_request.hpp"

#include <map>
#include <string>

#include "trie_node.hpp"

HttpRequest::HttpRequest() {}
HttpRequest::HttpRequest(const char *raw_request) {
  std::size_t i = 0;
  std::size_t j = 0;

  // Parse method
  this->method_ = kMethodTrie.search(raw_request, ' ');
  for (; raw_request[i] != ' '; i++)
    ;
  if (this->method_ == NONE || raw_request[i] != ' ') {
    throw BadRequestException();
  }
  j = ++i;

  // Parse URI
  for (; raw_request[j] != ' '; j++) {
    if (raw_request[j] < '!' || raw_request[j] > '~') {
      throw BadRequestException();
    }
  };
  this->uri_ = std::string(raw_request + i, j - i);
  if (this->uri_.empty() || raw_request[j] != ' ') {
    throw BadRequestException();
  }
  i = ++j;

  // Parse version_
  for (; raw_request[j] != '\r'; j++)
    ;
  this->version_ = std::string(raw_request + i, j - i);
  if (this->version_.empty() || raw_request[j + 1] != '\n' ||
      this->version_ != "HTTP/1.1") {
    throw BadRequestException();
  }
  i = j + 2;

  // Parse headers
  while (raw_request[i] != '\r' || raw_request[i + 1] != '\n') {
    j = i;
    for (; raw_request[j] != ':'; j++)
      ;
    std::string key = std::string(raw_request + i, j - i);
    i = j + 2;  // Skip ": "
    j = i;
    for (; raw_request[j] != '\r'; j++)
      ;
    std::string value = std::string(raw_request + i, j - i);
    this->headers_[key] = value;
    i = j + 2;  // Skip "\r\n"
  }
  i += 2;  // Skip the final "\r\n" after headers_

  // Parse body_
  j = i;
  for (; raw_request[j] != '\0'; j++)
    ;
  this->body_ = std::string(raw_request + i, j - i);
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

const char *HttpRequest::BadRequestException::what() const throw() {
  return "Bad request";
}

HttpMethod HttpRequest::get_method() const { return this->method_; }
const std::string &HttpRequest::get_uri() const { return this->uri_; }
const std::string &HttpRequest::get_version() const { return this->version_; }
const std::map<std::string, std::string> &HttpRequest::get_header() const {
  return this->headers_;
}
const std::string &HttpRequest::get_header(const std::string &key) const {
  return this->headers_.at(key);
}
const std::string &HttpRequest::get_body() const { return this->body_; }
