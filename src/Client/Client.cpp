#include "Client.hpp"

// static const int MAX_REQUEST_ENTITY_ = 4096;

Client::Client() {}

Client::Client(const Server& serv) {
	this->server_ptr_ = &serv;
}

// accept()の返り値をsetして使う
void Client::setSockfd(const int& sockfd) {
	this->sockfd_ = sockfd;
}

const int& Client::getSockfd() const {
	return this->sockfd_;
}
