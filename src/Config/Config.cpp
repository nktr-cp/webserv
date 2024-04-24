#include "Config.hpp"

Config::Config(const std::string& filename) {
	std::ifstream file(filename);

	if (!file.is_open()) {
		throw std::runtime_error("Failed to open file: " + filename);
	}

	std::string line;
	while (std::getline(file, line)) {
		size_t pos = line.find_first_of("#");
		if (pos == std::string::npos) {
			this->content_.append(line);
		} else {
			std::string add_line = line.substr(0, pos);
			this->content_.append(add_line);
		}
	}
	for (size_t i = 0; i < this->content_.size(); i++) {
		if (this->content_[i] == '\t') {
			this->content_[i] = ' ';
		}
	}
	this->parse();
}

void Config::create_sockets() {
	for (size_t i=0; i<servers_.size(); i++) {
		this->servers_[i].create_socket();
	}
}

// この処理を行うには、クライアントが必須
// void Config::prepare_monitor() {
// 	fd_set reads, writes;
// 	// fd_setのクリア
// 	FD_ZERO(&reads); FD_ZERO(&writes);

// 	for (size_t i=0; i<servers_.size(); i++) {

// 	}
// }

void Config::close_sockets() {
	for (size_t i=0; i<servers_.size(); i++) {
		close(this->servers_[i].getSockfd());
	}
}
