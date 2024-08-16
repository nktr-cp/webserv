#include "http_request.hpp"
#include "trie_node.hpp"
#include <string>
#include <map>
#include <iostream>

HttpRequest::HttpRequest() {}
HttpRequest::HttpRequest(const char *raw_request) {
    std::size_t i = 0;
    std::size_t j = 0;

    // Parse method
    this->method = method_trie.search(raw_request, ' ');
    for (; raw_request[i] != ' '; i++);
    if (this->method == NONE || raw_request[i] != ' ') {
        throw BadRequestException();
    }
    j = ++i;

    // Parse URI
    for (; raw_request[j] != ' '; j++) {
        if (raw_request[j] < '!' || raw_request[j] > '~') {
            throw BadRequestException();
        }
    };
    this->uri = std::string(raw_request + i, j - i);
    if (this->uri.empty() || raw_request[j] != ' ') {
        throw BadRequestException();
    }
    i = ++j;

    // Parse version
    for (; raw_request[j] != '\r'; j++);
    this->version = std::string(raw_request + i, j - i);
    if (this->version.empty() || raw_request[j + 1] != '\n'|| this->version != "HTTP/1.1") {
        throw BadRequestException();
    }
    i = j + 2;

    // Parse headers
    while (raw_request[i] != '\r' || raw_request[i + 1] != '\n') {
        j = i;
        for (; raw_request[j] != ':'; j++);
        std::string key = std::string(raw_request + i, j - i);
        i = j + 2; // Skip ": "
        j = i;
        for (; raw_request[j] != '\r'; j++);
        std::string value = std::string(raw_request + i, j - i);
        this->headers[key] = value;
        i = j + 2; // Skip "\r\n"
    }
    i += 2; // Skip the final "\r\n" after headers

    // Parse body
    j = i;
    for (; raw_request[j] != '\0'; j++);
    this->body = std::string(raw_request + i, j - i);
}
HttpRequest::HttpRequest(const HttpRequest &src) {
    *this = src;
}
HttpRequest &HttpRequest::operator=(const HttpRequest &src) {
    if (this != &src) {
        this->method = src.method;
        this->uri = src.uri;
        this->version = src.version;
        this->headers = src.headers;
        this->body = src.body;
    }
    return *this;
}
HttpRequest::~HttpRequest() {
}

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
const TrieNode<HttpMethod> HttpRequest::method_trie = initialize_method_trie();

const char *HttpRequest::BadRequestException::what() const throw() {
    return "Bad request";
}

HttpMethod HttpRequest::get_method() const {
    return this->method;
}
const std::string &HttpRequest::get_uri() const {
    return this->uri;
}
const std::string &HttpRequest::get_version() const {
    return this->version;
}
const std::map<std::string, std::string> &HttpRequest::get_header() const {
    return this->headers;
}
const std::string &HttpRequest::get_header(const std::string &key) const {
    return this->headers.at(key);
}
const std::string &HttpRequest::get_body() const {
    return this->body;
}
