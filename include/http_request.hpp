#pragma once
#include <cstddef>
#include <map>
#include <string>

#include "trie_node.hpp"
#include "typedefs.hpp"

enum HttpMethod {
  NONE = 0,
  GET,
  HEAD,
  POST,
  OPTIONS,
  PUT,
  DELETE,
};

std::string to_string(HttpMethod method);

class HttpRequest {
 private:
  static const TrieNode<HttpMethod> kMethodTrie;
  static const int kMaxHeaderSize;
  static const int kMaxPayloadSize;
  static const int kMaxUriSize;
  HttpMethod method_;
  std::string uri_;
  dict query_;
  std::string host_name_;
  std::string host_port_;
  std::string version_;
  dict headers_;
  std::string body_;
  int content_length_;

  const char *parse_method(const char *req);
  const char *parse_uri(const char *req);
  const char *parse_version(const char *req);
  const char *parse_header(const char *req);
  void parse_body(const char *req);

 public:
  HttpRequest();
  HttpRequest(const char *raw_request);
  HttpRequest(const HttpRequest &src);
  HttpRequest &operator=(const HttpRequest &src);
  ~HttpRequest();

  HttpMethod get_method() const;
  const std::string &get_uri() const;
  const dict &get_query() const;
  const std::string &get_query(const std::string &key) const;
  const std::string &get_host_name() const;
  const std::string &get_host_port() const;
  const std::string &get_version() const;
  const dict &get_header() const;
  const std::string &get_header(const std::string &key) const;
  const std::string &get_body() const;

  class RequestException : public std::exception {
    private:
      HttpStatus http_status_;
      char *message_;
    public:
      RequestException(HttpStatus http_status);
      RequestException(HttpStatus http_status, const char *message);
      const char *what() const throw();
      const HttpStatus get_status() const;
  };
  class BadRequestException : public RequestException {
    public:
      BadRequestException();
  };
  class PayloadTooLargeException : public RequestException {
    public:
      PayloadTooLargeException();
  };
  class UriTooLongException : public RequestException {
    public:
      UriTooLongException();
  };
  class RequestHeaderFieldsTooLargeException : public RequestException {
    public:
      RequestHeaderFieldsTooLargeException();
  };
  class InternalServerErrorException : public RequestException {
    public:
      InternalServerErrorException();
  };
};
