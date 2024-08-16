#pragma once
#include "trie_node.hpp"
#include <string>
#include <unordered_map>

enum HttpMethod {
    GET,
    HEAD,
    POST,
    OPTIONS,
    PUT,
    DELETE,
    NONE = 0,
};

class HttpRequest {
    private:
    static const TrieNode<HttpMethod> method_trie;
    HttpMethod method;
    std::string uri;
    std::string version;
    std::unordered_map<std::string, std::string> headers;
    std::string body;

    public:
    HttpRequest();
    HttpRequest(const char *raw_request);
    HttpRequest(const HttpRequest &src);
    HttpRequest &operator=(const HttpRequest &src);
    ~HttpRequest();

    HttpMethod get_method() const;
    const std::string &get_uri() const;
    const std::string &get_version() const;
    const std::unordered_map<std::string, std::string> &get_header() const;
    const std::string &get_header(const std::string &key) const;
    const std::string &get_body() const;

    class BadRequestException : public std::exception {
        public:
        const char *what() const throw();
    };
};
