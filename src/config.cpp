#include "config.hpp"

bool Config::isDirective(const std::string &token) {
  if (token == SERVER_DIRECTIVE || token == LOCATION_DIRECTIVE ||
      token == ERROR_PAGE_DIRECTIVE || token == MAX_BODY_SIZE_DIRECTIVE ||
      token == HOST_DIRECTIVE || token == PORT_NUMBER_DIRECTIVE ||
      token == SERVER_NAME_DIRECTIVE || token == METHODS_DIRECTIVE ||
      token == ROOT_DIRECTORY_DIRECTIVE || token == AUTOINDEX_DIRECTIVE ||
      token == CGI_DIRECTIVE || token == INDEX_DIRECTIVE ||
      token == EXTENSIONS_DIRECTIVE || token == UPLOAD_PATH_DIRECTIVE ||
      token == REDIRECT_DIRECTIVE)
    return true;
  return false;  // 最低の書き方、setを使おうとしたがクラッシュを直せなかったので暫定
}

static std::string tokenize(const std::string &content) {
  static size_t pos = 0;
  size_t start = pos;
  size_t end = 0;

  while (content[pos] && strchr(WIHTESPACE, content[pos])) pos++;
  start = pos;
  if (content[pos] == '\0') return "";
  if (strchr(SPECIAL_LETTERS, content[pos])) {
    pos++;
    return content.substr(start, 1);
  }
  while (content[pos] && !strchr(WIHTESPACE, content[pos]) &&
         !strchr(SPECIAL_LETTERS, content[pos]))
    pos++;
  end = pos;
  return content.substr(start, end - start);
}

void Config::parse() {
  std::string token;

  while (true) {
    token = tokenize(content_);
    if (token.empty()) break;
    if (token == "server")
      this->parseServer();
    else
      throw SyntaxError(token);
  }
}

void Config::parseServer() {
  std::string token;
  ServerConfig server_config = ServerConfig();

  token = tokenize(content_);
  if (token != "{") throw SyntaxError(token);
  while (true) {
    token = tokenize(content_);
    if (token == ERROR_PAGE_DIRECTIVE)
      this->parseError(&server_config);
    else if (token == MAX_BODY_SIZE_DIRECTIVE)
      this->parseMaxBody(&server_config);
    else if (token == HOST_DIRECTIVE)
      this->parseHost(&server_config);
    else if (token == PORT_NUMBER_DIRECTIVE)
      this->parsePortNumber(&server_config);
    else if (token == SERVER_NAME_DIRECTIVE)
      this->parseServerName(&server_config);
    else if (token == LOCATION_DIRECTIVE)
      this->parseLocation(&server_config);
    else
      break;
  }
  if (token != "}") throw SyntaxError(token);
  this->serverConfigs_.push_back(server_config);
}

void Config::parseLocation(ServerConfig *server) {
  std::string token;
  Location location = Location();

  token = tokenize(content_);
  if (token[0] != '/' || token[token.size() - 1] != '/')
    throw SyntaxError(token);
  location.setName(token);
  token = tokenize(content_);
  if (token != "{") throw SyntaxError(token);
  while (true) {
    token = tokenize(content_);
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
    else if (token == CGI_DIRECTIVE)
      this->parseCgiPath(&location);
    else
      break;
  }
  if (token != "}") throw SyntaxError(token);
  server->addLocation(location);
}

void Config::parseError(ServerConfig *server) {
  std::string token;
  int code;

  token = tokenize(content_);
  try {
    unsigned int range[] = {100, 599};
    code = ft::stoui(token, range);
  } catch (std::invalid_argument &e) {
    throw InvalidArgument(token);
  } catch (std::out_of_range &e) {
    throw ArgOutOfRange(token);
  }
  token = tokenize(content_);
  if (token.find(HTTP) != 0 && token.find(HTTPS) != 0 && token[0] != '/')
    throw InvalidArgument(token);
  server->addError(code, token);
  token = tokenize(content_);
  if (token != ";") throw SyntaxError(token);
}

void Config::parseMaxBody(ServerConfig *server) {
  std::string token;
  unsigned int max_body = 0;

  token = tokenize(content_);
  try {
    unsigned int range[] = {0, MAX_BODY_SIZE};
    max_body = ft::stoui(token, range);
  } catch (std::invalid_argument &e) {
    throw InvalidArgument(token);
  } catch (std::out_of_range &e) {
    throw ArgOutOfRange(token);
  }
  server->setMaxBodySize(max_body);
  token = tokenize(content_);
  if (token != ";") throw SyntaxError(token);
}

void Config::parseHost(ServerConfig *server) {
  std::string token;

  token = tokenize(content_);
  server->setHost(token);
  token = tokenize(content_);
  if (token != ";") throw SyntaxError(token);
}

void Config::parsePortNumber(ServerConfig *server) {
  std::string token;

  token = tokenize(content_);
  try {
    unsigned int range[] = {0, 65535};
    ft::stoui(token, range);
  } catch (std::invalid_argument &e) {
    throw InvalidArgument(token);
  } catch (std::out_of_range &e) {
    throw ArgOutOfRange(token);
  }
  server->setPort(token);
  token = tokenize(content_);
  if (token != ";") throw SyntaxError(token);
}

void Config::parseServerName(ServerConfig *server) {
  std::string token;

  token = tokenize(content_);
  server->setServerName(token);
  token = tokenize(content_);
  if (token != ";") throw SyntaxError(token);
}

void Config::parseMethods(Location *location) {
  std::string token;

  while (true) {
    token = tokenize(content_);
    if (token == ";") break;
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

  token = tokenize(content_);
  location->setRoot(token);
  token = tokenize(content_);
  if (token != ";") throw SyntaxError(token);
}

void Config::parseAutoIndex(Location *location) {
  std::string token;

  token = tokenize(content_);
  if (token == "on")
    location->setAutoindex(true);
  else if (token == "off")
    location->setAutoindex(false);
  else
    throw SyntaxError(token);
  token = tokenize(content_);
  if (token != ";") throw SyntaxError(token);
}

void Config::parseIndex(Location *location) {
  std::string token;

  while (true) {
    token = tokenize(content_);
    if (token == ";") break;
    if (isDirective(token)) throw SyntaxError(token);
    location->addIndex(token);
  }
}

void Config::parseExtensions(Location *location) {
  std::string token;

  token = tokenize(content_);
  location->addExtension(token);
  token = tokenize(content_);
  if (token != ";") throw SyntaxError(token);
}

void Config::parseUploadPath(Location *location) {
  std::string token;

  token = tokenize(content_);
  location->setUploadPath(token);
  token = tokenize(content_);
  if (token != ";") throw SyntaxError(token);
}

void Config::parseRedirect(Location *location) {
  std::string token;

  token = tokenize(content_);
  location->setRedirect(token);
  token = tokenize(content_);
  if (token != ";") throw SyntaxError(token);
}

void Config::parseCgiPath(Location *location) {
  std::string token;

  token = tokenize(content_);
  location->setCgiPath(token);
  token = tokenize(content_);
  if (token != ";") throw SyntaxError(token);
}

Config::Config(const std::string &filename) {
  std::ifstream file(filename.c_str());

  if (!file.is_open()) {
    throw std::runtime_error("Failed to open file: " + filename);
  }

  std::string line;
  while (std::getline(file, line)) {
    size_t pos = line.find_first_of("#");
    if (pos == std::string::npos) {
      content_.append(line);
    } else {
      std::string add_line = line.substr(0, pos);
      content_.append(add_line);
    }
  }
  this->parse();
  // print locations
  // for (size_t i = 0; i < this->serverConfigs_.size(); i++) {
  //   std::vector<Location> locations = this->serverConfigs_[i].getLocations();
  //   for (size_t j = 0; j < locations.size(); j++) {
  //     locations[j].print();
  //   }
  // }
}

std::vector<ServerConfig> Config::getServerConfigs() const {
  return this->serverConfigs_;
}