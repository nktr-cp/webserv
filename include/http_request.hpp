#pragma once
#include <climits>
#include <cstddef>
#include <map>
#include <string>

#include "trie_node.hpp"
#include "typedefs.hpp"
#include "utils.hpp"

class HttpRequest {
 private:
  static const TrieNode<HttpMethod> kMethodTrie;
  static const size_t kMaxHeaderSize;
  static const size_t kMaxPayloadSize;
  static const size_t kMaxUriSize;
  HttpMethod method_;
  std::string uri_;
  dict query_;
  std::string hostName_;
  std::string hostPort_;
  std::string version_;
  dict headers_;
  std::string body_;
  size_t contentLength_;

  const char* parseMethod(const char* req);
  const char* parseUri(const char* req);
  const char* parseVersion(const char* req);
  const char* parseHeader(const char* req);

 public:
  HttpRequest();
  HttpRequest(const char* raw_request);
  HttpRequest(const HttpRequest& src);
  HttpRequest& operator=(const HttpRequest& src);
  ~HttpRequest();

  HttpMethod getMethod() const;
  const std::string& getUri() const;
  const dict& getQuery() const;
  const std::string& getQuery(const std::string& key) const;
  const std::string& getQueryAsStr() const;
  const std::string& getHostName() const;
  const std::string& getHostPort() const;
  const std::string& getVersion() const;
  const dict& getHeader() const;
  const std::string& getHeader(const std::string& key) const;
  const std::string& getBody() const;
};
