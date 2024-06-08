#ifndef REQUEST_HPP_
#define REQUEST_HPP_

#include <map>
#include <sstream>
#include <string>

typedef std::map<std::string, std::string> dict;

class Request {
 private:
  std::string method_;
  std::string path_;
  dict headers_;
  std::string body_;

  void parse(std::string raw_request_);

 public:
  Request(std::string raw_request_);

  std::string get_method() const;
  std::string get_path() const;
  dict get_headers() const;
  std::string get_body() const;
};

#endif  // REQUEST_HPP_
