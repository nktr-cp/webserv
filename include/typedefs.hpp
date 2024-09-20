#ifndef TYPEDEFS_HPP
#define TYPEDEFS_HPP

#define MAX_BODY_SIZE 1000000

#include <map>
#include <vector>
#include <string>
#include <iostream>

#include "typedefs.hpp"

#define GATEWAY_TIMEOUT_SECONDS 10
#define FRIENDLY_ERROR_PAGE_LENGTH 512

class VersionInfo {
 public:
  static const std::string kProgramName;
  static const std::string kProgramVersion;
  static const std::string kHttpVersion;
  static const std::string kCgiVersion;
  static const std::string kUrlPrefixSecure;
  static const std::string kUrlPrefix;
};

enum SocketType { kServer, kClient };

typedef std::map<std::string, std::string> dict;
typedef std::pair<SocketType, int> SocketPair;

enum HttpStatus {
  CONTINUE = 100,
  SWITCHING_PROTOCOLS = 101,
  PROCESSING = 102,
  EARLY_HINTS = 103,
  OK = 200,
  CREATED = 201,
  ACCEPTED = 202,
  NON_AUTHORITATIVE_INFORMATION = 203,
  NO_CONTENT = 204,
  RESET_CONTENT = 205,
  PARTIAL_CONTENT = 206,
  MULTOPLE_CHOICES = 300,
  MOVED_PERMANENTLY = 301,
  FOUND = 302,
  SEE_OTHER = 303,
  NOT_MODIFIED = 304,
  USE_PROXY = 305,
  TEMPORARY_REDIRECT = 307,
  PERMANENT_REDIRECT = 308,
  BAD_REQUEST = 400,
  UNAUTHORIZED = 401,
  PAYMENT_REQUIRED = 402,
  FORBIDDEN = 403,
  NOT_FOUND = 404,
  METHOD_NOT_ALLOWED = 405,
  NOT_ACCEPTABLE = 406,
  PROXY_AUTH_REQUIRED = 407,
  REQUEST_TIMEOUT = 408,
  CONFLICT = 409,
  GONE = 410,
  LENGTH_REQUIRED = 411,
  PRECONDITION_FAILED = 412,
  PAYLOAD_TOO_LARGE = 413,
  URI_TOO_LONG = 414,
  UNSUPPORTED_MEDIA_TYPE = 415,
  RANGE_NOT_SATISFIABLE = 416,
  EXPECTATION_FAILED = 417,
  MISDIRECTED_REQUEST = 421,
  UNPROCESSABLE_CONTENT = 422,
  TOO_EARLY = 425,
  UPGRADE_REQUIRED = 426,
  PRECONDITION_REQUIRED = 428,
  TOO_MANY_REQUESTS = 429,
  REQUEST_HEADER_FIELDS_TOO_LARGE = 431,
  UNAVAILABLE_FOR_LEGAL_REASONS = 451,
  INTERNAL_SERVER_ERROR = 500,
  NOT_IMPLEMENTED = 501,
  BAD_GATEWAY = 502,
  SERVICE_UNAVAILABLE = 503,
  GATEWAY_TIMEOUT = 504,
  HTTP_VERSION_NOT_SUPPORTED = 505,
  INSUFFICIENT_STORAGE = 507,
  LOOP_DETECTED = 508,
  NOT_EXTENDED = 510,
};

enum HttpMethod {
  NONE = 0,
  GET = 1,
  HEAD = 2,
  POST = 4,
  PUT = 8,
  DELETE = 16,
  CONNECT = 32,
  OPTIONS = 64,
  TRACE = 128,
  PATCH = 256,
};

#endif  // TYPEDEFS_HPP