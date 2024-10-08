#ifndef CONFIG_HPP_
#define CONFIG_HPP_

#include <cstring>
#include <fstream>
#include <map>
#include <string>

#include "http_request.hpp"
#include "location.hpp"
#include "typedefs.hpp"
#include "utils.hpp"

#define WIHTESPACE " \t\n"
#define SPECIAL_LETTERS "{};"

#define SERVER_DIRECTIVE "server"
#define LOCATION_DIRECTIVE "location"
#define ERROR_PAGE_DIRECTIVE "errorpage"
#define MAX_BODY_SIZE_DIRECTIVE "maxbody"
#define HOST_DIRECTIVE "host"
#define PORT_NUMBER_DIRECTIVE "listen"
#define SERVER_NAME_DIRECTIVE "server_name"
#define METHODS_DIRECTIVE "method"
#define ROOT_DIRECTORY_DIRECTIVE "root"
#define AUTOINDEX_DIRECTIVE "autoindex"
#define CGI_DIRECTIVE "cgi_path"
#define INDEX_DIRECTIVE "index"
#define EXTENSIONS_DIRECTIVE "extension"
#define UPLOAD_PATH_DIRECTIVE "uploadpath"
#define REDIRECT_DIRECTIVE "redirect"

class ServerConfig {
 private:
  std::map<HttpStatus, std::string> errors_;
  std::vector<Location> locations_;
  std::string host_;
  std::string port_;
  std::string serverName_;
  int maxBodySize_;

 public:
  ServerConfig();

  void setHost(const std::string &);
  void setPort(const std::string &);
  void setServerName(const std::string &);
  void setMaxBodySize(int);

  void addError(int, const std::string &);
  void addLocation(const Location &);

  const std::string &getHost() const;
  const std::string &getPort() const;
  const std::map<HttpStatus, std::string> &getErrors() const;
  const std::string &getErrorPage(HttpStatus) const;
  const std::vector<Location> &getLocations() const;
  const std::string &getServerName() const;
  int getMaxBodySize() const;
};

class Config {
 private:
  std::string content_;
  std::vector<ServerConfig> serverConfigs_;
  bool isDirective(const std::string &token);

  void parse();
  void parseServer();
  void parseError(ServerConfig *server);
  void parseMaxBody(ServerConfig *server);
  void parseHost(ServerConfig *server);
  void parsePortNumber(ServerConfig *server);
  void parseServerName(ServerConfig *server);

  void parseLocation(ServerConfig *server);
  void parseMethods(Location *location);
  void parseRoot(Location *location);
  void parseAutoIndex(Location *location);
  void parseIndex(Location *location);
  void parseExtensions(Location *location);
  void parseUploadPath(Location *location);
  void parseRedirect(Location *location);
  void parseCgiPath(Location *location);

 public:
  Config(const std::string &filename);

  std::vector<ServerConfig> getServerConfigs() const;
};

#endif  // CONFIG_HPP_
