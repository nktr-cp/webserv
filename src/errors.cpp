#include "utils.hpp"

ExtraErrors::ExtraErrors(const std::string& msg) : std::runtime_error(msg) {
  std::cerr << msg << std::endl;
}

const std::string ExtraErrors::ProgramNamePrefix = VersionInfo::kProgramName + std::string(": ");

SyntaxError::SyntaxError(const std::string& token)
    : ExtraErrors(ErrMsgWrapper(token)) {}

const std::string SyntaxError::ErrorMessage =
    "syntax error near unexpected token ";

std::string SyntaxError::ErrMsgWrapper(const std::string& token) {
  return this->ProgramNamePrefix + this->ErrorMessage + std::string("`") +
         token + std::string("'");
}

SysCallFailed::SysCallFailed(const std::string &scname)
    : ExtraErrors(ErrMsgWrapper(scname)) {}

const std::string SysCallFailed::ErrorMessage = strerror(errno);

std::string SysCallFailed::ErrMsgWrapper(const std::string& arg) {
  return this->ProgramNamePrefix + arg + std::string(": ") + this->ErrorMessage;
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