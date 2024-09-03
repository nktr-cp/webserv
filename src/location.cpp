#include "location.hpp"

Location::Location()
    : methods_(0),
      root_(""),
      autoindex_(false),
      index_(),
      extensions_(),
      upoadPath_(""),
      redirect_("") {}

void Location::addMethod(const HttpMethod method) {
  this->methods_ |= static_cast<short>(method);
}

void Location::setName(const std::string& name) { this->name_ = name; }

void Location::setRoot(const std::string& root) { this->root_ = root; }

void Location::setAutoindex(bool autoindex) { this->autoindex_ = autoindex; }

void Location::addIndex(const std::string& index) { this->index_.push_back(index); }

void Location::addExtension(const std::string& extension) {
  this->extensions_.push_back(extension);
}

void Location::setUploadPath(const std::string& uploadPath) {
  this->upoadPath_ = uploadPath;
}

void Location::setRedirect(const std::string& redirect) {
  this->redirect_ = redirect;
}

void Location::setCgiPath(const std::string& cgiPath) {
  this->cgiPath_ = cgiPath;
}

short Location::getMethods() const { return this->methods_; }

const std::string& Location::getName() const { return this->name_; }

const std::string& Location::getRoot() const { return this->root_; }

bool Location::isAutoIndex() const { return this->autoindex_; }

const std::vector<std::string>& Location::getIndex() const { return this->index_; }

const std::vector<std::string>& Location::getExtensions() const {
  return this->extensions_;
}

const std::string& Location::getUploadPath() const {
  return this->upoadPath_;
}

const std::string& Location::getRedirect() const {
  return this->redirect_;
}

const std::string& Location::getCgiPath() const {
  return this->cgiPath_;
}

bool Location::isCgi() const {
  return !this->cgiPath_.empty();
}

void Location::print() {
  std::cout << "Name: " << this->name_ << std::endl;
  std::cout << "Methods: " << this->methods_ << std::endl;
  std::cout << "Root: " << this->root_ << std::endl;
  std::cout << "Autoindex: " << this->autoindex_ << std::endl;
  std::cout << "Index: ";
  for (size_t i = 0; i < this->index_.size(); i++) {
    std::cout << this->index_[i] << " ";
  }
  std::cout << std::endl;
  std::cout << "Extensions: ";
  for (size_t i = 0; i < this->extensions_.size(); i++) {
    std::cout << this->extensions_[i] << " ";
  }
  std::cout << std::endl;
  std::cout << "Upload path: " << this->upoadPath_ << std::endl;
  std::cout << "Redirect: " << this->redirect_ << std::endl;
  std::cout << "Is CGI: " << this->isCgi() << std::endl;
  std::cout << "Cgi path: " << this->cgiPath_ << std::endl;
}