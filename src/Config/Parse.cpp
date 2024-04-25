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
		if (token == ERROR_PAGE_DIRECTIVE)
			this->parseError(&server);
		else if (token == MAX_BODY_SIZE_DIRECTIVE)
			this->parseMaxBody(&server);
		else if (token == HOST_DIRECTIVE)
			this->parseHost(&server);
		else if (token == PORT_NUMBER_DIRECTIVE)
			this->parsePortNumber(&server);
		else if (token == SERVER_NAME_DIRECTIVE)
			this->parseServerName(&server);
		else if (token == LOCATION_DIRECTIVE)
			this->parseLocation(&server);
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
	token = tokenize(this->content_);
	if (token != "{")
		throw SyntaxError(token);
	while (true) {
		token = tokenize(this->content_);
		if (token == METHODS_DIRECTIVE)
			this->parseMethods(&location);
		else if (token == ROOT_DIRECTORY_DIRECTIVE)
			this->parseRoot(&location);
		else if (token == AUTOINDEX_DIRECTIVE)
			this->parseAutoIndex(&location);
		else if (token == INDEX_DIRECTIVE)
			this->parseIndex(&location);
		else if (token == EXTENSIONS_DIRECTIVE)
			this->parseExtensions(&location);
		else if (token == UPLOAD_PATH_DIRECTIVE)
			this->parseUploadPath(&location);
		else if (token == REDIRECT_DIRECTIVE)
			this->parseRedirect(&location);
		else
			break;
	}
	if (token != "}")
		throw SyntaxError(token);
	server->addLocation(location);
}

void	Config::parseError(Server *server) {
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
	token = tokenize(this->content_);
	server->addError(code, token);
	token = tokenize(this->content_);
	if (token != ";")
		throw SyntaxError(token);
}

void Config::parseMaxBody(Server *server) {
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
	server->setMaxBodySize(max_body);
	token = tokenize(this->content_);
	if (token != ";")
		throw SyntaxError(token);
}

void Config::parseHost(Server *server) {
	std::string token;

	token = tokenize(this->content_);
	server->setHost(token);
	token = tokenize(this->content_);
	if (token != ";")
		throw SyntaxError(token);
}

void Config::parsePortNumber(Server *server) {
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
	server->setPort(token);
	token = tokenize(this->content_);
	if (token != ";")
		throw SyntaxError(token);
}

void Config::parseServerName(Server *server) {
	std::string token;

	token = tokenize(this->content_);
	server->setServerName(token);
	token = tokenize(this->content_);
	if (token != ";")
		throw SyntaxError(token);
}

void Config::parseMethods(Location *location) {
	std::string token;

	while (true) {
		token = tokenize(this->content_);
		if (token == ";")
			break;
		if (token == "GET")
			location->addMethod(GET);
		else if (token == "POST")
			location->addMethod(POST);
		else if (token == "DELETE")
			location->addMethod(DELETE);
		else
			throw SyntaxError(token);
	}
}

void Config::parseRoot(Location *location) {
	std::string token;

	token = tokenize(this->content_);
	location->setRoot(token);
	token = tokenize(this->content_);
	if (token != ";")
		throw SyntaxError(token);
}

void Config::parseAutoIndex(Location *location) {
	std::string token;

	token = tokenize(this->content_);
	if (token == "on")
		location->setAutoindex(true);
	else if (token == "off")
		location->setAutoindex(false);
	else
		throw SyntaxError(token);
	token = tokenize(this->content_);
	if (token != ";")
		throw SyntaxError(token);
}

void Config::parseIndex(Location *location) {
	std::string token;

	token = tokenize(this->content_);
	location->setIndex(token);
	token = tokenize(this->content_);
	if (token != ";")
		throw SyntaxError(token);
}

void Config::parseExtensions(Location *location) {
	std::string token;

	token = tokenize(this->content_);
	location->addExtension(token);
	token = tokenize(this->content_);
	if (token != ";")
		throw SyntaxError(token);
}

void Config::parseUploadPath(Location *location) {
	std::string token;

	token = tokenize(this->content_);
	location->setUploadPath(token);
	token = tokenize(this->content_);
	if (token != ";")
		throw SyntaxError(token);
}

void Config::parseRedirect(Location *location) {
	std::string token;

	token = tokenize(this->content_);
	location->setRedirect(token);
	token = tokenize(this->content_);
	if (token != ";")
		throw SyntaxError(token);
}
