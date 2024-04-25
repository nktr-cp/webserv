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
	if (strchr(SPECIAL_LETTERS, content[pos]))
	{
		pos++;
		return content.substr(start, 1);
	}
	while (content[pos] && !strchr(WIHTESPACE, content[pos]) && !strchr(SPECIAL_LETTERS, content[pos]))
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
			throw SyntaxError(token);
	}
}

void Config::parseServer() {
	std::string token;
	Server server = Server();

	token = tokenize(this->content_);
	if (token != "{")
		throw SyntaxError(token);
	while (true) {
		token = tokenize(this->content_);
		if (token == "listen")
			this->parseListen(&server);
		else if (token == "server_name")
			this->parseHost(&server);
		else if (token == "error_page")
			this->parseError();
		else if (token == "location")
			this->parseLocation(&server);
		else if (token == "root")
			this->parseRoot();
		else
			break;
	}
	if (token != "}")
		throw SyntaxError(token);
	this->servers_.push_back(server);
}

void Config::parseLocation(Server *server) {
	std::string token;
	Location location = Location();

	token = tokenize(this->content_);
	if (token[0] != '/' || token[token.size() - 1] != '/')
		throw SyntaxError(token);
	std::cout << "Location: " << token << std::endl;
	token = tokenize(this->content_);
	if (token != "{")
		throw SyntaxError(token);
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
		throw SyntaxError(token);
	server->addLocation(location);
}

void Config::parseListen(Server *server) {
	std::string token;
	int port = 0;

	token = tokenize(this->content_);
	try {
		port = ft::stoui(token, (unsigned int[2]){0, 65535});
	} catch (std::invalid_argument& e) {
		throw InvalidArgument(token);
	} catch (std::out_of_range& e) {
		throw ArgOutOfRange(token);
	}
	std::cout << "Port: " << port << std::endl;
	server->setPort(token);
	token = tokenize(this->content_);
	if (token != ";")
		throw SyntaxError(token);
}

void Config::parseRoot() {
	std::string token;

	token = tokenize(this->content_);
	std::cout << "Root: " << token << std::endl;
	token = tokenize(this->content_);
	if (token != ";")
		throw SyntaxError(token);
}

void Config::parseHost(Server *server) {
	std::string token;

	token = tokenize(this->content_);
	std::cout << "Host: " << token << std::endl;
	server->setHost(token);
	token = tokenize(this->content_);
	if (token != ";")
		throw SyntaxError(token);
}

void Config::parseError() {
	std::string token;

	token = tokenize(this->content_);
	std::cout << "Error: " << token << std::endl;
	token = tokenize(this->content_);
	if (token != ";")
		throw SyntaxError(token);
}

void Config::parseMaxBody() {
	std::string token;
	unsigned int max_body = 0;

	token = tokenize(this->content_);
	try {
		max_body = ft::stoui(token, (unsigned int[2]){0, MAX_BODY_SIZE});
	} catch (std::invalid_argument& e) {
		throw InvalidArgument(token);
	} catch (std::out_of_range& e) {
		throw ArgOutOfRange(token);
	}
	std::cout << "Max body: " << max_body << std::endl;
	token = tokenize(this->content_);
	if (token != ";")
		throw SyntaxError(token);
}

void Config::parseIndex() {
	std::string token;

	token = tokenize(this->content_);
	std::cout << "Index: " << token << std::endl;
	token = tokenize(this->content_);
	if (token != ";")
		throw SyntaxError(token);
}

void Config::parseAutoIndex() {
	std::string token;

	token = tokenize(this->content_);
	if (token == "on")
		std::cout << "Autoindex: on" << std::endl;
	else if (token == "off")
		std::cout << "Autoindex: off" << std::endl;
	else
		throw SyntaxError(token);
	token = tokenize(this->content_);
	if (token != ";")
		throw SyntaxError(token);
}

void Config::parseDirList() {
	std::string token;

	token = tokenize(this->content_);
	if (token == "on")
		std::cout << "Dir list: on" << std::endl;
	else if (token == "off")
		std::cout << "Dir list: off" << std::endl;
	else
		throw SyntaxError(token);
	token = tokenize(this->content_);
	if (token != ";")
		throw SyntaxError(token);
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
			throw SyntaxError(token);
	}
}

void Config::parseExtension() {
	std::string token;

	token = tokenize(this->content_);
	std::cout << "Extension: " << token << std::endl;
	token = tokenize(this->content_);
	if (token != ";")
		throw SyntaxError(token);
}

void Config::parseReturn() {
	std::string token;
	int code;

	token = tokenize(this->content_);
	try {
		code = ft::stoui(token, (unsigned int[2]){100, 599});
	} catch (std::invalid_argument& e) {
		throw InvalidArgument(token);
	} catch (std::out_of_range& e) {
		throw ArgOutOfRange(token);
	}
	std::cout << "Code: " << code << ", ";
	token = tokenize(this->content_);
	std::cout << "Return to " << token << std::endl;
	token = tokenize(this->content_);
	if (token != ";")
		throw SyntaxError(token);
}