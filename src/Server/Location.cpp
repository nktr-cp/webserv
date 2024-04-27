#include "Location.hpp"

Location::Location():
	methods_(0),
	root_(""),
	autoindex_(false),
	index_(""),
	extensions_(),
	upload_path_(""),
	redirect_("") {}

void Location::setUri(const std::string& uri) {
	this->uri_ = uri;
}

void Location::addMethod(const HttpMethod method) {
	this->methods_ |= static_cast<short>(method);
}

void Location::setRoot(const std::string& root) {
	this->root_ = root;
}

void Location::setAutoindex(bool autoindex) {
	this->autoindex_ = autoindex;
}

void Location::setIndex(const std::string& index) {
	this->index_ = index;
}

void Location::addExtension(const std::string& extension) {
	this->extensions_.push_back(extension);
}

void Location::setUploadPath(const std::string& upload_path) {
	this->upload_path_ = upload_path;
}

void Location::setRedirect(const std::string& redirect) {
	this->redirect_ = redirect;
}

const std::string& Location::getUri() const {
	return this->uri_;
}

short Location::getMethods() const {
	return this->methods_;
}

const std::string& Location::getRoot() const {
	return this->root_;
}

bool Location::getAutoindex() const {
	return this->autoindex_;
}

const std::string& Location::getIndex() const {
	return this->index_;
}

const std::vector<std::string>& Location::getExtensions() const {
	return this->extensions_;
}

const std::string& Location::getUploadPath() const {
	return this->upload_path_;
}

const std::string& Location::getRedirect() const {
	return this->redirect_;
}

void Location::print() {
	std::cout << "URI: " << this->uri_ << std::endl;
	std::cout << "Methods: " << this->methods_ << std::endl;
	std::cout << "Root: " << this->root_ << std::endl;
	std::cout << "Autoindex: " << this->autoindex_ << std::endl;
	std::cout << "Index: " << this->index_ << std::endl;
	std::cout << "Extensions: ";
	for (size_t i = 0; i < this->extensions_.size(); i++) {
		std::cout << this->extensions_[i] << " ";
	}
	std::cout << std::endl;
	std::cout << "Upload path: " << this->upload_path_ << std::endl;
	std::cout << "Redirect: " << this->redirect_ << std::endl;
}
