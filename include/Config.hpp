#ifndef CONFIG_HPP_
#define CONFIG_HPP_

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "Server.hpp"
#include "Client.hpp"
#include "Utils.hpp"
#include "Errors.hpp"

#define WIHTESPACE " \t\n"
#define SPECIAL_LETTERS "{};"

#define MAX_BODY_SIZE 1000000

class Config {
	private:
		std::string content_;
		std::vector<Server> servers_;

		fd_set readfds_, writefds_;
		std::vector<Server> servers_;
		std::vector<Client> clients_;

	public:
		Config(const std::string& filename);

		// parse
		void parse();
		void parseServer();
		void parseLocation(Server *server);
		void parseListen(Server *server);
		void parseRoot();
		void parseHost(Server *server);
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

		//debug
		void printServers();
};

#endif // CONFIG_HPP_
