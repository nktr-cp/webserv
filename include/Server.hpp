#ifndef SERVER_HPP_
#define SERVER_HPP_

#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <map>

#include "Location.hpp"
#include "Errors.hpp"

class Server {
	private:
		//parsed
		std::string host_;
		std::string port_;
		std::vector<Location> locations_;
		std::string server_name_;
		std::string root_;
		std::string index_;
		std::map<int, std::string> errors_;
		bool autoindex_;
		int max_body_size_;

		//utils
		int sockfd_;
		static const unsigned int backlog_;

	public:
		// constructor, destructor
		Server();

		// setter, getter
		void setHost(const std::string&);
		void setPort(const std::string&);
		void setServerName(const std::string&);
		void setRoot(const std::string&);
		void setIndex(const std::string&);
		void setAutoIndex(bool);

		void addError(int, const std::string&);
		void addLocation(const Location&);

		const std::string& getHost() const;
		const std::string& getPort() const;

		// server method
		void create_socket();

		//debug
		void print();
};

#endif // SERVER_HPP_