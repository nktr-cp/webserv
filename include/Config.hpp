#ifndef CONFIG_HPP_
#define CONFIG_HPP_

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "Server.hpp"
#include "Client.hpp"
#include "Typedefs.hpp"

#define WIHTESPACE " \t\n"
#define SPECIAL_LETTERS "{};"

#define SERVER_DIRECTIVE "server"
#define LOCATION_DIRECTIVE "location"
#define ERROR_PAGE_DIRECTIVE "errorpage"
#define MAX_BODY_SIZE_DIRECTIVE "maxbody"
#define HOST_DIRECTIVE "host"
#define PORT_NUMBER_DIRECTIVE "listen"
#define SERVER_NAME_DIRECTIVE "server_name"
#define METHODS_DIRECTIVE "method"
#define ROOT_DIRECTORY_DIRECTIVE "root"
#define AUTOINDEX_DIRECTIVE "autoindex"
#define INDEX_DIRECTIVE "index"
#define EXTENSIONS_DIRECTIVE "extension"
#define UPLOAD_PATH_DIRECTIVE "uploadpath"
#define REDIRECT_DIRECTIVE "redirect"

class Config {
	private:
		std::string content_;

		fd_set readfds_, writefds_;
		std::vector<Server> servers_;
		std::vector<Client> clients_;

	public:
		Config(const std::string& filename);

		// parse
		void	parse();
		void	parseServer();
		void	parseError(Server *server);
		void	parseMaxBody(Server *server);
		void	parseHost(Server *server);
		void	parsePortNumber(Server *server);
		void	parseServerName(Server *server);

		void	parseLocation(Server *server);
		void	parseMethods(Location *location);
		void	parseRoot(Location *location);
		void	parseAutoIndex(Location *location);
		void	parseIndex(Location *location);
		void	parseExtensions(Location *location);
		void	parseUploadPath(Location *location);
		void	parseRedirect(Location *location);

		// server method
		void set_select();
		void accept_sockets();
		void get_request();
		void event_loop();
		void create_sockets();
		void close_sockets();
		//debug
		void printServers();
};

#endif // CONFIG_HPP_
