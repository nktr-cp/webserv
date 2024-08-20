#include "utils.hpp"

template <typename T>
Result<T>::Result(T value) : value(value), error(false), msg("") {}

template <typename T>
Result<T>::Result(const std::string& error_msg) : value(T()), error(true), msg(error_msg) {}

template <typename T>
bool Result<T>::isOk() const {
  return !error;
}

template <typename T>
T Result<T>::getValue() const {
  return value;
}

template <typename T>
std::string &Result<T>::getMsg() {
  return msg;
}

template <typename T>
Result<T> &Result<T>::operator=(const T &rhs) {
  value = rhs;
  return *this;
}

template <typename T>
Result<T> Ok(T value) {
  return Result<T>(value);
}

template <typename T>
Result<T> Ko(const std::string& msg) {
  return Result<T>(msg);
}