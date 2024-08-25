#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <cstring>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>

// Alternative error handing method
template <typename T>
class Result {
 private:
  T value;
  bool error;
  std::string msg;

 public:
  Result(T value);
  Result(const std::string& error_msg);
  Result& operator=(const T& rhs);

  bool isOk() const;
  T getValue() const;
  std::string& getMsg();
};

template <typename T>
Result<T> Ok(T value);

template <typename T>
Result<T> Ko(const std::string& msg);

// util functions
namespace ft {
  bool isNumber(const std::string& str);
  unsigned int stoui(const std::string& str, const unsigned int range[2]);
  std::string uitost(unsigned int n);
}  // namespace ft

namespace filemanip {
  Result<bool> isFile(const std::string& path);
  Result<bool> isDir(const std::string& path);
  Result<bool> pathExists(const std::string& path);
  Result<bool> isDeletable(const std::string& path);
}

// error class
#include <cerrno>
#include <stdexcept>
#include <string>

/*
** ExtraErrors class is a base class for all custom exceptions.
** It inherits from std::runtime_error and has a virtual method ErrMsgWrapper.
** This method is implemented in derived classes and is used to format the error
*message.
** The constructor of ExtraErrors takes a string as an argument and passes it to
*the std::runtime_error constructor.
** The ProgramNamePrefix is a constant string that is used to prefix the error
*message.
*/
class ExtraErrors : public std::runtime_error {
 public:
  ExtraErrors(const std::string& msg);
  static const std::string ProgramNamePrefix;

 private:
  virtual std::string ErrMsgWrapper(const std::string& msg) = 0;
};

/*
** SyntaxError class is a derived class of ExtraErrors.
** It is used to throw an exception when a syntax error is encountered.
** The constructor of SyntaxError takes a string as an argument and passes it to
*the ExtraErrors constructor.
** The ErrorMessage is a constant string that is used to format the error
*message.
** The ErrMsgWrapper method is used to format the error message.
** Example error message: "webserv: syntax error near unexpected token `token'"
*/
class SyntaxError : public ExtraErrors {
 public:
  SyntaxError(const std::string& msg);
  static const std::string ErrorMessage;

 private:
  std::string ErrMsgWrapper(const std::string& token);
};

/*
** SysCallFailed class is a derived class of ExtraErrors.
** It is used to throw an exception when a system call fails.
** The constructor of SysCallFailed takes no arguments and passes the error
*message to the ExtraErrors constructor.
** The ErrorMessage is a constant string that is used to format the error
*message.
** The ErrMsgWrapper method is used to format the error message.
** Example error message: "webserv: fork: system call failed"
*/
class SysCallFailed : public ExtraErrors {
 public:
  SysCallFailed(void);
  static const std::string ErrorMessage;

 private:
  std::string ErrMsgWrapper(const std::string& msg);
};

/*
** ArgOutOfRange class is a derived class of ExtraErrors.
** It is used to throw an exception when an argument is out of range.
** The constructor of ArgOutOfRange takes a string as an argument and passes it
*to the ExtraErrors constructor.
** The ErrorMessage is a constant string that is used to format the error
*message.
** The ErrMsgWrapper method is used to format the error message.
** Example error message: "webserv: 1000000: argument out of range"
*/
class ArgOutOfRange : public ExtraErrors {
 public:
  ArgOutOfRange(const std::string& arg);
  static const std::string ErrorMessage;

 private:
  std::string ErrMsgWrapper(const std::string& arg);
};

class InvalidArgument : public ExtraErrors {
 public:
  InvalidArgument(const std::string& arg);
  static const std::string ErrorMessage;

 private:
  std::string ErrMsgWrapper(const std::string& arg);
};

#include "result.tpp"

#endif  // UTILS_HPP_