#ifndef CONFIG_HPP_
#define CONFIG_HPP_

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

class Config {
	private:
		std::string content_;

	public:
		Config(const std::string& filename);
		// void parse();
};

#endif // CONFIG_HPP_
