#ifndef CONFIG_HPP_
#define CONFIG_HPP_

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "Utils.hpp"
#include "Errors.hpp"

#define WIHTESPACE " \t\n"
#define SPECIAL_LETTERS "{};"

class Config {
	private:
		std::string content_;

	public:
		Config(const std::string& filename);
		void parse();
		void parseServer();
		void parseLocation();
		void parseListen();
		void parseRoot();
		void parseHost();
		void parseServerName();
		void parseError();
		void parseMaxBody();
		void parseIndex();
		void parseAutoIndex();
		void parseDirList();
		void parseMethods();
		void parseExtension();
		void parseReturn();
};

#endif // CONFIG_HPP_
