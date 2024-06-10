#include "config.hpp"

ServerConfig::ServerConfig()
    : host_(""), port_(""), server_name_(""), max_body_size_(MAX_BODY_SIZE) {}

void ServerConfig::setHost(const std::string& host) { this->host_ = host; }

void ServerConfig::setPort(const std::string& port) { this->port_ = port; }

void ServerConfig::setServerName(const std::string& server_name) {
  this->server_name_ = server_name;
}

void ServerConfig::setMaxBodySize(int max_body_size) {
  this->max_body_size_ = max_body_size;
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

const std::vector<Location>& ServerConfig::getLocations() const {
  return this->locations_;
}

const std::string& ServerConfig::getHost() const { return this->host_; }

const std::string& ServerConfig::getPort() const { return this->port_; }

const std::string& ServerConfig::getServerName() const {
  return this->server_name_;
}

int ServerConfig::getMaxBodySize() const { return this->max_body_size_; }
