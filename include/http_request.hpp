#pragma once
#include <climits>
#include <cstddef>
#include <map>
#include <string>

#include "trie_node.hpp"
#include "typedefs.hpp"
#include "utils.hpp"

#define DEFAULT_PORT "8080"

class HttpRequest
{
private:
  std::string buffer_;
  static const TrieNode<HttpMethod> kMethodTrie;
  static const size_t kMaxHeaderSize;
  static const size_t kMaxUriSize;
  HttpMethod method_;
  std::string uri_;
  dict query_;
  std::string hostName_;
  std::string hostPort_;
  std::string version_;
  dict headers_;
  std::string body_;
  long contentLength_;

  const char *parseMethod(const char *req);
  const char *parseUri(const char *req);
  const char *parseVersion(const char *req);
  const char *parseHeader(const char *req);

public:
  enum ParseTarget
  {
    HEADER = 0,
    BODY,
    DONE
  };
  static const size_t kMaxPayloadSize;
  bool keepAlive;
  ParseTarget progress;

  HttpRequest();
  HttpRequest(const HttpRequest &src);
  HttpRequest &operator=(const HttpRequest &src);
  ~HttpRequest();

  void parseRequest(const char *payload);
  HttpMethod getMethod() const;
  const std::string &getUri() const;
  const dict &getQuery() const;
  const std::string &getQuery(const std::string &key) const;
  const std::string &getQueryAsStr() const;
  const std::string &getHostName() const;
  const std::string &getHostPort() const;
  const std::string &getVersion() const;
  const dict &getHeader() const;
  const std::string &getHeader(const std::string &key) const;
  const std::string &getBody() const;
};
