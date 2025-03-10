#include "utils.hpp"

namespace http {
std::string statusToString(HttpStatus status) {
  switch (status) {
    case OK:
      return "OK";
    case FOUND:
      return "Found";
    case TEMPORARY_REDIRECT:
      return "Temporary Redirect";
    case PERMANENT_REDIRECT:
      return "Permanent Redirect";
    case BAD_REQUEST:
      return "Bad Request";
    case UNAUTHORIZED:
      return "Unauthorized";
    case FORBIDDEN:
      return "Forbidden";
    case NOT_FOUND:
      return "Not Found";
    case METHOD_NOT_ALLOWED:
      return "Method Not Allowed";
    case LENGTH_REQUIRED:
      return "Length Required";
    case PAYLOAD_TOO_LARGE:
      return "Payload Too Large";
    case URI_TOO_LONG:
      return "URI Too Long";
    case TOO_MANY_REQUESTS:
      return "Too Many Requests";
    case REQUEST_HEADER_FIELDS_TOO_LARGE:
      return "Header Fields Too Large";
    case INTERNAL_SERVER_ERROR:
      return "Internal Server Error";
    case NOT_IMPLEMENTED:
      return "Not Implemented";
    case BAD_GATEWAY:
      return "Bad Gateway";
    case SERVICE_UNAVAILABLE:
      return "Service Unavailable";
    case GATEWAY_TIMEOUT:
      return "Gateway Timeout";
    case CREATED:
      return "Created";
    case NO_CONTENT:
      return "No Content";
    case MOVED_PERMANENTLY:
      return "Moved Permanently";
    case SEE_OTHER:
      return "See Other";
    case NOT_MODIFIED:
      return "Not Modified";
    case USE_PROXY:
      return "Use Proxy";
    default:
      return "I'm a teapot";
  }
}

responseStatusException::responseStatusException(HttpStatus status)
    : std::runtime_error(ft::uitost(status) + " " + statusToString(status)),
      status_(status) {}

HttpStatus responseStatusException::getStatus() const { return this->status_; }

std::string methodToString(HttpMethod method) {
  switch (method) {
    case GET:
      return "GET";
    case HEAD:
      return "HEAD";
    case POST:
      return "POST";
    case PUT:
      return "PUT";
    case DELETE:
      return "DELETE";
    case CONNECT:
      return "CONNECT";
    case OPTIONS:
      return "OPTIONS";
    case TRACE:
      return "TRACE";
    case PATCH:
      return "PATCH";
    default:
      return "NONE";
  }
}
}  // namespace http