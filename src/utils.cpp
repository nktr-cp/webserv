#include "utils.hpp"

// util functions
namespace ft {
/*
 * isNumber: check if a string is composed purely of digits
 * @param str: string to check
 * @return: true if str is composed purely of digits, false otherwise
 */
bool isNumber(const std::string& str) {
  for (size_t i = 0; i < str.size(); i++) {
    if (!isdigit(str[i])) return false;
  }
  return true;
}

/*
 * stoui: convert a string to an unsigned integer
 * @param str: string to convert
 * @param range: range of the integer, inclusive
 * @return: the integer represented by str
 * @throw invalid_argument: if str is not composed purely of digits
 * @throw out_of_range: if the integer is out of range
 */
unsigned int stoui(const std::string& str, const unsigned int range[2]) {
  unsigned n = 0;

  if (!ft::isNumber(str) || range[0] > range[1])
    throw std::invalid_argument("stoui: invalid argument");
  for (size_t i = 0; i < str.size(); i++) {
    n = n * 10 + str[i] - '0';
  }
  if (n < range[0] || range[1] < n)
    throw std::out_of_range("stoui: out of range");
  return n;
}

std::string uitost(unsigned int n) {
  std::string str = "";
  if (n == 0) return "0";
  while (n) {
    str = (char)(n % 10 + '0') + str;
    n /= 10;
  }
  return str;
}
}  // namespace ft

namespace filemanip {
Result<bool> isFile(const std::string& path) {
  struct stat buf;
  if (stat(path.c_str(), &buf) != 0) return Ko<bool>(strerror(errno));
  return Ok<bool>(S_ISREG(buf.st_mode));
}

Result<bool> isDir(const std::string& path) {
  struct stat buf;
  if (stat(path.c_str(), &buf) != 0) return Ko<bool>(strerror(errno));
  return Ok<bool>(S_ISDIR(buf.st_mode));
}

Result<bool> pathExists(const std::string& path) {
  struct stat buf;
  if (stat(path.c_str(), &buf) == 0) return Ok<bool>(true);
  if (errno == ENOENT) return Ok<bool>(false);
  return Ko<bool>(strerror(errno));
}

Result<bool> isDeletable(const std::string& path) {
  struct stat buf;
  std::string path_copy = path;
  std::string dir_name = dirname(&path_copy[0]);
  if (stat(dir_name.c_str(), &buf) != 0) {
    return Ko<bool>(strerror(errno));
  }
  if (access(dir_name.c_str(), W_OK | X_OK) != 0) {
    return Ko<bool>(strerror(errno));
  }
  return Ok<bool>(true);
}

}  // namespace filemanip

const std::string VersionInfo::kProgramName = "webserv";
const std::string VersionInfo::kHttpVersion = "HTTP/1.1";
const std::string VersionInfo::kCgiVersion = "CGI/1.1";
const std::string VersionInfo::kUrlPrefixSecure = "https://";
const std::string VersionInfo::kUrlPrefix = "http://";