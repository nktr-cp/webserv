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
		// TODO
		Response();
};

#endif // RESPONSE_HPP_
