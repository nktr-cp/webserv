#ifndef CGI_HANDLER_HPP_
#define CGI_HANDLER_HPP_

#include <string>

#include "location.hpp"
#include "request.hpp"
#include "response.hpp"

class CGIHandler {
 private:
  std::string script_path_;
  Location location_;

 public:
  Response execute(const Request&);
};

#endif  // CGI_HANDLER_HPP_
