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
}  // namespace ft

// error class
ExtraErrors::ExtraErrors(const std::string& msg) : std::runtime_error(msg) {}

const std::string ExtraErrors::ProgramNamePrefix = "webserv: ";

SyntaxError::SyntaxError(const std::string& token)
    : ExtraErrors(ErrMsgWrapper(token)) {}

const std::string SyntaxError::ErrorMessage =
    "syntax error near unexpected token ";

std::string SyntaxError::ErrMsgWrapper(const std::string& token) {
  return this->ProgramNamePrefix + this->ErrorMessage + std::string("`") +
         token + std::string("'");
}

SysCallFailed::SysCallFailed(void)
    : ExtraErrors(ErrMsgWrapper(strerror(errno))) {}

const std::string SysCallFailed::ErrorMessage = ": system call failed";

std::string SysCallFailed::ErrMsgWrapper(const std::string& arg) {
  return this->ProgramNamePrefix + arg + this->ErrorMessage;
}

ArgOutOfRange::ArgOutOfRange(const std::string& arg)
    : ExtraErrors(ErrMsgWrapper(arg)) {}

const std::string ArgOutOfRange::ErrorMessage = ": argument out of range";

std::string ArgOutOfRange::ErrMsgWrapper(const std::string& arg) {
  return this->ProgramNamePrefix + arg + this->ErrorMessage;
}

InvalidArgument::InvalidArgument(const std::string& arg)
    : ExtraErrors(ErrMsgWrapper(arg)) {}

const std::string InvalidArgument::ErrorMessage = ": invalid argument";

std::string InvalidArgument::ErrMsgWrapper(const std::string& arg) {
  return this->ProgramNamePrefix + arg + this->ErrorMessage;
}
