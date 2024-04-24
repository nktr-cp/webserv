#ifndef SERVER_HPP_
#define SERVER_HPP_

#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>

#include "Location.hpp"

class Server {
	private:
		std::string host_;
		std::string port_;
		std::vector<Location> locations_;

		int sockfd_;
		static const unsigned int backlog_;
	public:
		// constructor, destructor
		Server();

		// setter, getter
		void setHost(const std::string&);
		void setPort(const std::string&);
		void addLocation(const Location&);

		const std::string& getHost() const;
		const std::string& getPort() const;

		// server method
		void create_socket();
};

#endif // SERVER_HPP_