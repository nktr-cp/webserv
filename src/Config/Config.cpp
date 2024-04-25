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
	this->parse();
	//debug
	std::printf("\n\n=====================\n\n");
	this->printServers();
}

void Config::printServers() {
	for (size_t i = 0; i < this->servers_.size(); i++) {
		std::cout << "=====================" << std::endl << "Server " << i + 1 << ":" << std::endl;
		this->servers_[i].print();
	}
}

