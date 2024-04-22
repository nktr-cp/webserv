#include "Config.hpp"

Config::Config(const std::string& filename) {
	std::ifstream file(filename);

	if (!file.is_open()) {
		throw std::runtime_error("Failed to open file: " + filename);
	}

	std::stringstream buf;
	buf << file.rdbuf();

	this->content_ = buf.str();
}
