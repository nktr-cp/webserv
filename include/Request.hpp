#ifndef REQUEST_HPP_
#define REQUEST_HPP_

#include <string>
#include <sstream>
#include "Typedefs.hpp"

class Request {
	private:
		std::string method_;
		std::string path_;
		dict headers_;
		std::string body_;
	public:
		Request(std::string row_request);

		std::string get_method() const;
		std::string get_path() const;
		dict get_headers() const;
		std::string get_body() const;
};

#endif // REQUEST_HPP_
