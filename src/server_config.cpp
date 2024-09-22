#include "config.hpp"

ServerConfig::ServerConfig()
    : host_(""), port_(""), serverName_(""), maxBodySize_(MAX_BODY_SIZE) {}

void ServerConfig::setHost(const std::string& host) { this->host_ = host; }

void ServerConfig::setPort(const std::string& port) { this->port_ = port; }

void ServerConfig::setServerName(const std::string& serverName) {
  this->serverName_ = serverName;
}

void ServerConfig::setMaxBodySize(int maxBodySize) {
  this->maxBodySize_ = maxBodySize;
}

void ServerConfig::addLocation(const Location& location) {
  this->locations_.push_back(location);
}

void ServerConfig::addError(int status, const std::string& path) {
  this->errors_.insert(std::pair<HttpStatus, std::string>(
      static_cast<HttpStatus>(status), path));
}

const std::map<HttpStatus, std::string>& ServerConfig::getErrors() const {
  return this->errors_;
}

const std::string& ServerConfig::getErrorPage(HttpStatus status) const {
  std::map<HttpStatus, std::string>::const_iterator it =
      this->errors_.find(status);
  if (it != this->errors_.end()) {
    return it->second;
  } else {
    static const std::string empty = "";
    return empty;
  }
}

const std::vector<Location>& ServerConfig::getLocations() const {
  return this->locations_;
}

const std::string& ServerConfig::getHost() const { return this->host_; }

const std::string& ServerConfig::getPort() const { return this->port_; }

const std::string& ServerConfig::getServerName() const {
  return this->serverName_;
}

int ServerConfig::getMaxBodySize() const { return this->maxBodySize_; }
