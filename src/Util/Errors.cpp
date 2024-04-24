#include "Errors.hpp"

ExtraErrors::ExtraErrors(const std::string& msg) : std::runtime_error(msg) {}

std::string SyntaxError::ErrMrgWrapper(const std::string& token) {
	return SYNTAX_ERROR + std::string("`") + token + std::string("'");
}

SyntaxError::SyntaxError(const std::string& token) : ExtraErrors(ErrMrgWrapper(token)) {}

std::string SysCallFailed::ErrMrgWrapper(const std::string& msg) {
	return PROGRAM_NAME_PREFIX + msg;
}

SysCallFailed::SysCallFailed(void) : ExtraErrors(ErrMrgWrapper(strerror(errno))) {}