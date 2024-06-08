#ifndef REQUEST_HPP_
#define REQUEST_HPP_

#include <map>
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
};

#endif  // REQUEST_HPP_
