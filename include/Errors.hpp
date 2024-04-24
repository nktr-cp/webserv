#ifndef ERRORS_HPP_
#define ERRORS_HPP_

#include <stdexcept>
#include <string>
#include <cerrno>

#define PROGRAM_NAME_PREFIX std::string("webserv: ")

#define SYNTAX_ERROR PROGRAM_NAME_PREFIX + std::string("syntax error near unexpected token ")

class ExtraErrors : public std::runtime_error {
	public:
		ExtraErrors(const std::string& msg);

	private:
		virtual std::string ErrMrgWrapper(const std::string& msg) = 0;
};

class SyntaxError : public ExtraErrors {
	public:
		SyntaxError(const std::string& msg);
	
	private:
		std::string ErrMrgWrapper(const std::string& token);
};

class SysCallFailed : public ExtraErrors {
	public:
		SysCallFailed(void);
	
	private:
		std::string ErrMrgWrapper(const std::string& msg);
};

#endif
