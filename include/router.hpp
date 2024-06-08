#ifndef ROUTER_HPP_
#define ROUTER_HPP_

#include <string>

#include "route.hpp"

class Router {
 private:
  std::vector<Route> routes_;

 public:
  Route findRoute(std::string path);
};

#endif  // ROUTER_HPP_