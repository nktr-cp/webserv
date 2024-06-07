#ifndef REQUEST_HPP_
#define REQUEST_HPP_

#include <string>
#include "Typedefs.hpp"

class Request {
	private:
		std::string method_;
		std::string path_;
		dict headers_;
		std::string body_;
	public:
		Request(std::string row_request);
};

#endif // REQUEST_HPP_
