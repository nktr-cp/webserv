#ifndef RESPONSE_HPP_
#define RESPONSE_HPP_

typedef std::map<std::string, std::string> dict;

class Response {
 private:
  int status_code_;
  dict headers_;
  std::string body_;

 public:
  Response(int, dict, std::string);
};

#endif  // RESPONSE_HPP_
