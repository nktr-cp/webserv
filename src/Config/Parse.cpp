#include "Config.hpp"

static std::string tokenize(std::string& content) {
	static size_t pos = 0;
	size_t start = pos;
	size_t end = 0;

	while (content[pos] && strchr(WIHTESPACE, content[pos]))
		pos++;
	start = pos;
	if (content[pos] == '\0')
		return "";
	if (strchr(SPECCHARS, content[pos]))
	{
		pos++;
		return content.substr(start, 1);
	}
	while (content[pos] && !strchr(WIHTESPACE, content[pos]) && !strchr(SPECCHARS, content[pos]))
		pos++;
	end = pos;
	return content.substr(start, end - start);
}

void Config::parse() {
	std::string token;

	while (true) {
		token = tokenize(this->content_);
		if (token.empty())
			break;
		if (token == "server")
			this->parseServer();
		else
			throw std::runtime_error(EM_CONFIG);
	}
}

void Config::parseServer() {
	std::string token;

	token = tokenize(this->content_);
	if (token != "{")
		throw std::runtime_error(EM_CONFIG);
	while (true) {
		token = tokenize(this->content_);
		if (token == "listen")
			this->parseListen();
		else if (token == "server_name")
			this->parseServerName();
		else if (token == "error_page")
			this->parseError();
		else if (token == "location")
			this->parseLocation();
		else if (token == "root")
			this->parseRoot();
		else
			break;
	}
	if (token != "}")
		throw std::runtime_error(EM_CONFIG);
}

void Config::parseLocation() {
	std::string token;

	token = tokenize(this->content_);
	if (token[0] != '/' || token[token.size() - 1] != '/')
		throw std::runtime_error(EM_CONFIG);
	std::cout << "Location: " << token << std::endl;
	token = tokenize(this->content_);
	if (token != "{")
		throw std::runtime_error(EM_CONFIG);
	while (true) {
		token = tokenize(this->content_);
		if (token == "root")
			this->parseRoot();
		else if (token == "index")
			this->parseIndex();
		else if (token == "autoindex")
			this->parseAutoIndex();
		else if (token == "method")
			this->parseMethods();
		else if (token == "max_body")
			this->parseMaxBody();
		else if (token == "dir_list")
			this->parseDirList();
		else if (token == "extension")
			this->parseExtension();
		else if (token == "return")
			this->parseReturn();
		else
			break;
	}
	if (token != "}")
		throw std::runtime_error(EM_CONFIG);
}

void Config::parseListen() {
	std::string token;
	int port = 0;

	token = tokenize(this->content_);
	if (isNumber(token))
		port = std::stoi(token);
	else
		throw std::runtime_error(EM_CONFIG);
	if (port < 0 || port > 65535)
		throw std::runtime_error(EM_CONFIG);
	std::cout << "Port: " << port << std::endl;
	token = tokenize(this->content_);
	if (token != ";")
		throw std::runtime_error(EM_CONFIG);
}

void Config::parseRoot() {
	std::string token;

	token = tokenize(this->content_);
	std::cout << "Root: " << token << std::endl;
	token = tokenize(this->content_);
	if (token != ";")
		throw std::runtime_error(EM_CONFIG);
}

void Config::parseHost() {
	std::string token;

	token = tokenize(this->content_);
	std::cout << "Host: " << token << std::endl;
	token = tokenize(this->content_);
	if (token != ";")
		throw std::runtime_error(EM_CONFIG);
}

void Config::parseServerName() {
	std::string token;

	token = tokenize(this->content_);
	std::cout << "Server name: " << token << std::endl;
	token = tokenize(this->content_);
	if (token != ";")
		throw std::runtime_error(EM_CONFIG);
}

void Config::parseError() {
	std::string token;

	token = tokenize(this->content_);
	std::cout << "Error: " << token << std::endl;
	token = tokenize(this->content_);
	if (token != ";")
		throw std::runtime_error(EM_CONFIG);
}

void Config::parseMaxBody() {
	std::string token;
	int max_body = 0;

	token = tokenize(this->content_);
	if (isNumber(token))
		max_body = std::stoi(token);
	else
		throw std::runtime_error(EM_CONFIG);
	if (max_body < 0)
		throw std::runtime_error(EM_CONFIG);
	std::cout << "Max body: " << max_body << std::endl;
	token = tokenize(this->content_);
	if (token != ";")
		throw std::runtime_error(EM_CONFIG);
}

void Config::parseIndex() {
	std::string token;

	token = tokenize(this->content_);
	std::cout << "Index: " << token << std::endl;
	token = tokenize(this->content_);
	if (token != ";")
		throw std::runtime_error(EM_CONFIG);
}

void Config::parseAutoIndex() {
	std::string token;

	token = tokenize(this->content_);
	if (token == "on")
		std::cout << "Autoindex: on" << std::endl;
	else if (token == "off")
		std::cout << "Autoindex: off" << std::endl;
	else
		throw std::runtime_error(EM_CONFIG);
	token = tokenize(this->content_);
	if (token != ";")
		throw std::runtime_error(EM_CONFIG);
}

void Config::parseDirList() {
	std::string token;

	token = tokenize(this->content_);
	if (token == "on")
		std::cout << "Dir list: on" << std::endl;
	else if (token == "off")
		std::cout << "Dir list: off" << std::endl;
	else
		throw std::runtime_error(EM_CONFIG);
	token = tokenize(this->content_);
	if (token != ";")
		throw std::runtime_error(EM_CONFIG);
}

void Config::parseMethods() {
	std::string token;

	while (true) {
		token = tokenize(this->content_);
		if (token == ";")
			break;
		if (token == "GET")
			std::cout << "Method: GET" << std::endl;
		else if (token == "POST")
			std::cout << "Method: POST" << std::endl;
		else if (token == "DELETE")
			std::cout << "Method: DELETE" << std::endl;
		else
			throw std::runtime_error(EM_CONFIG);
	}
}

void Config::parseExtension() {
	std::string token;

	token = tokenize(this->content_);
	std::cout << "Extension: " << token << std::endl;
	token = tokenize(this->content_);
	if (token != ";")
		throw std::runtime_error(EM_CONFIG);
}

void Config::parseReturn() {
	std::string token;
	int code;

	token = tokenize(this->content_);
	if (isNumber(token))
		code = std::stoi(token);
	else
		throw std::runtime_error(EM_CONFIG);
	if (code < 100 || code > 599)
		throw std::runtime_error(EM_CONFIG);
	std::cout << "Code: " << code << ", ";
	token = tokenize(this->content_);
	std::cout << "Return to " << token << std::endl;
	token = tokenize(this->content_);
	if (token != ";")
		throw std::runtime_error(EM_CONFIG);
}