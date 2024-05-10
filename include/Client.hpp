#ifndef CLIENNT_HPP_
#define CLIENNT_HPP_

#include <string>
#include "Server.hpp"

class Client {
	private:
		static const int MAX_REQUEST_ENTITY_;
		int sockfd_;
		const Server* server_ptr_;

	public:
		Client();
		Client(const Server&);

		void setSockfd(const int&);
		const int& getSockfd() const;

};

#endif // CLIENT_HPP_