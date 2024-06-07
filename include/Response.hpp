#ifndef RESPONSE_HPP_
#define RESPONSE_HPP_

#include <string>
#include "Typedefs.hpp"

class Response {
	private:
		int status_code_;
		dict headers_;
		std::string body_;
	public:
		Response(int, dict, std::string);

		int get_status_code() const;
		dict get_headers() const;
		std::string get_body() const;
};

#endif // RESPONSE_HPP_
