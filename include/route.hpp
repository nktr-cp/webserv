#ifndef ROUTE_HPP_
#define ROUTE_HPP_

#include <string>
#include <vector>

#include "location.hpp"
#include "request.hpp"
#include "response.hpp"

class Route {
 private:
  std::string path_;
  std::vector<std::string> methods_;
  Location location_;

 public:
  Response handleRequest(Request request);
};

#endif  // ROUTE_HPP_