#ifndef CACHE_HPP_
#define CACHE_HPP_

#include <string>

#include "request.hpp"
#include "response.hpp"

class Cache {
 private:
  dict cache_data_;

 public:
  Response get(std::string key) const;
  void set(std::string key, Response value);
};

#endif  // CACHE_HPP_
