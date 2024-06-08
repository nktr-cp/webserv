#ifndef IO_MANAGER_HPP_
#define IO_MANAGER_HPP_

#include <vector>

class IOManager {
 private:
  std::vector<int> fds_;

 public:
  void monitor();
  void add_fd(int);
  void remove_fd(int);
};

#endif  // IO_MANAGER_HPP_
