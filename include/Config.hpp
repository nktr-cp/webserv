#ifndef CONFIG_HPP_
#define CONFIG_HPP_

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "Server.hpp"
#include "Client.hpp"
#include "Util.hpp"

#define WIHTESPACE " \t\n"
#define SPECCHARS "{};"

#define EM_CONFIG "Invalid configuration file"

class Config {
	private:
		std::string content_;

		fd_set readfds_, writefds_;
		std::vector<Server> servers_;
		std::vector<Client> clients_;

	public:
		Config(const std::string& filename);

		// parse
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

		// server method
		void create_sockets();
		void prepare_monitor();
		void event_loop();
		void close_sockets();
};

#endif // CONFIG_HPP_
