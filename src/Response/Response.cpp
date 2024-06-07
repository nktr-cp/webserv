#include "Response.hpp"

Response::Response(int status_code, dict headers, std::string body)
: status_code_(status_code), headers_(headers), body_(body) {
	status_code_ = status_code;
}

int Response::get_status_code() const {
	return status_code_;
}

dict Response::get_headers() const {
	return headers_;
}

std::string Response::get_body() const {
	return body_;
}
