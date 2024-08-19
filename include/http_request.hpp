#pragma once
#include <cstddef>
#include <map>
#include <string>

#include "trie_node.hpp"

enum HttpMethod {
  NONE = 0,
  GET,
  HEAD,
  POST,
  OPTIONS,
  PUT,
  DELETE,
};

class HttpRequest {
 private:
  static const TrieNode<HttpMethod> kMethodTrie;
  HttpMethod method_;
  std::string uri_;
  std::string version_;
  std::map<std::string, std::string> headers_;
  std::string body_;

 public:
  HttpRequest();
  HttpRequest(const char *raw_request);
  HttpRequest(const HttpRequest &src);
  HttpRequest &operator=(const HttpRequest &src);
  ~HttpRequest();

  HttpMethod get_method() const;
  const std::string &get_uri() const;
  const std::string &get_version() const;
  const std::map<std::string, std::string> &get_header() const;
  const std::string &get_header(const std::string &key) const;
  const std::string &get_body() const;

  class BadRequestException : public std::exception {
   public:
    const char *what() const throw();
  };
};
