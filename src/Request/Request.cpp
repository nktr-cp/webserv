#include "Request.hpp"

Request::Request(std::string row_request) {
	std::istringstream iss(row_request);
	std::string line;

	// omit error handling so far
	if (std::getline(iss, line)) {
		char delim; // this should be '/'
		std::istringstream request_line_stream(line);
		request_line_stream >> method_ >> delim >> path_;
	} else {
		throw std::runtime_error("Empty request");
	}

	// parse headers
	while (std::getline(iss, line) && !line.empty()) {
		size_t pos = line.find(':');
		if (pos != std::string::npos) {
			std::string key = line.substr(0, pos);
			std::string value = line.substr(pos + 2);
			headers_[key] = value;
		}
	}

	// parse body
	if (headers_.find("Content-Length") != headers_.end()) {
		// ft::の関数使えなかったので(throwされる)、差し向きstd::stoi
		int content_length = std::stoi(headers_["Content-Length"]);
		body_.resize(content_length);
		iss.read(&body_[0], content_length);
	} else {
		// throw std::runtime_error("Content-Length header not found");
		body_ = "";
	}
}

std::string Request::get_method() const {
	return method_;
}

std::string Request::get_path() const {
	return path_;
}

dict Request::get_headers() const {
	return headers_;
}

std::string Request::get_body() const {
	return body_;
}
