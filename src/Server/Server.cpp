#include "Server.hpp"

const unsigned int Server::backlog_ = 10;

Server::Server():
	host_(""),
	port_(""),
	server_name_(""),
	max_body_size_(MAX_BODY_SIZE) {}

void Server::setHost(const std::string& host) {
	this->host_ = host;
}

void Server::setPort(const std::string& port) {
	this->port_ = port;
}

void Server::setServerName(const std::string& server_name) {
	this->server_name_ = server_name;
}

void Server::setMaxBodySize(unsigned int max_body_size) {
	this->max_body_size_ = max_body_size;
}

void Server::addLocation(const Location& location) {
	this->locations_.push_back(location);
}

void Server::addError(int status, const std::string& path) {
	this->errors_.insert(std::pair<HttpStatus, std::string>(static_cast<HttpStatus>(status), path));
}

const std::string& Server::getHost() const {
	return this->host_;
}

const std::string& Server::getPort() const {
	return this->port_;
}

const std::string& Server::getServerName() const {
	return this->server_name_;
}

unsigned int Server::getMaxBodySize() const {
	return this->max_body_size_;
}

void Server::create_socket() {
	// hint: ネットワークアドレスを解消するための参考情報をもつ
	// address: 返り値となる、完全なaddrinfo構造体
	struct addrinfo hints, *address;

	memset(&hints, 0, sizeof(hints));

	/*****************************************
	AF_INET: IPv4のアドレスファミリ
	(*アドレスファミリ: 通信で使用されるアドレス形式やプロトコル)
	SOCK_STREAM: TCPを利用して、シーケンス化された双方向接続のバイトストリームを達成できるソケット
	AI_PASSIVE: ソケットアドレスが、bind()呼び出し時に使われるようにする
	*****************************************/
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	// getaddrinfo()でaddress変数にアドレス情報を格納
	if (getaddrinfo(host_.c_str(), port_.c_str(), &hints, &address) != 0) {
		// throw ... # 適当な例外処理を投げるようにする
	}

	// addressの情報を元にソケットを作成
	sockfd_ = socket(address->ai_family, address->ai_socktype, address->ai_protocol);
	if (sockfd_ == -1) {
		// throw ... # 適当な例外処理を投げるようにする
	}

	// socketのファイル状態フラグを決める
	// 第三引数に指定されたO_NONBLOCKを設定
	fcntl(sockfd_, F_SETFL, O_NONBLOCK);

	// ソケットとアドレスの情報を関係付ける
	if (bind(sockfd_, address->ai_addr, address->ai_addrlen) == -1) {
		// throw ... # 適当な例外処理を投げるようにする
	}

	/*****************************************
  (この設定が必須かどうかは、よくわかってない)
	ソケットオプションの設定
	SOL_SOCKET ... ソケットオプションレベルでの指定
	SO_REUSEADDR ... すでに使用中のアドレスをバインドできるようになる
	optvalによって上の設定を有効にする
	*****************************************/
	int optval = 1;
	setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

	// クライアントからの接続要求待ちにする
	if (listen(sockfd_, Server::backlog_) == -1) {
		throw SysCallFailed();
	}
}

//debug
#include <iostream>
void Server::print() {
	std::cout << "host: " << host_ << std::endl;
	std::cout << "port: " << port_ << std::endl;
	std::cout << "server_name: " << server_name_ << std::endl;
	std::cout << "max_body_size: " << max_body_size_ << std::endl;
	std::cout << "errors: " << std::endl;
	for (std::map<HttpStatus, std::string>::iterator it = errors_.begin(); it != errors_.end(); it++) {
		std::cout << it->first << " " << it->second << std::endl;
	}
	std::cout << "locations: " << std::endl;
	for (std::vector<Location>::iterator it = locations_.begin(); it != locations_.end(); it++) {
		std::cout << "-----------------------" << std::endl;
		it->print();
	}
}