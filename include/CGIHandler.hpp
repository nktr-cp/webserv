#ifndef CGI_HANDLER_HPP_
#define CGI_HANDLER_HPP_

#include <string>
#include "Request.hpp"
#include "Response.hpp"

class CGIHandler {
	private:
		std::string script_path_;
	public:
		CGIHandler(std::string script_path);
		Response execute(Request& request);
};

#endif // CGI_HANDLER_HPP_
