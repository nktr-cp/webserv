#include "Config.hpp"

Config::Config(const std::string& filename) {
	std::ifstream file(filename);

	if (!file.is_open()) {
		throw std::runtime_error("Failed to open file: " + filename);
	}

	std::string line;
	while (std::getline(file, line)) {
		size_t pos = line.find_first_of("#");
		if (pos == std::string::npos) {
			this->content_.append(line);
		} else {
			std::string add_line = line.substr(0, pos);
			this->content_.append(add_line);
		}
	}
	this->parse();
	//debug
	std::printf("\n\n=====================\n\n");
	this->printServers();
}

void Config::printServers() {
	for (size_t i = 0; i < this->servers_.size(); i++) {
		std::cout << "=====================" << std::endl << "Server " << i + 1 << ":" << std::endl;
		this->servers_[i].print();
	}
}

void Config::set_select() {
	/***************************************************************
	// kqueue
	int kq = kqueue();
	if (kq == -1) {
		// throw # 適当なエラー
	}

	int nevents = 0;
	struct kevent events[2 * servers_.size()];

	for (size_t i=0; i<servers_.size(); i++) {
		EV_SET(&kev, ident, filter, flags, fflags, data, udata);
		EV_SET(&events[nevents++], servers_[i].getSockfd(), EVFILT_READ, EV_ADD, 0, 0, NULL);
	}

	kevent(int kq, const struct kevent *changelist, int nchanges,
         struct kevent *eventlist, int nevents,
         const struct timespec *timeout);
	int result = 0;
	if ((result = kevent(kq, events, nevents, NULL, 0, NULL)) == -1) {
		// throw # 適切なエラーを返す
	} else if (result == 0) {
		// throw # 適切なエラーを返す
	}

	close(kq);
	***************************************************************/

	int nfds = 0;
	// fd_setのクリア
	FD_ZERO(&readfds_); FD_ZERO(&writefds_);

	for (size_t i=0; i<servers_.size(); ++i) {
		// fd_setであるreadfdsにサーバーのソケットを追加
		FD_SET(servers_[i].getSockfd(), &readfds_);
		++nfds;
	}
	// 最初の段階ではここは逐次セットで良さそう
	for (size_t i=0; i<clients_.size(); ++i) {
		// readfdsにクライアントのソケットを追加
		FD_SET(clients_[i].getSockfd(), &readfds_);
		++nfds;
	}

	/********************************************************************
	select(int nfds, fd_set *restrict readfds, fd_set *restrict writefds,
         fd_set *restrict errorfds, struct timeval *restrict timeout);
	複数のファイルディスクリプタを監視する。
	一つ以上のファイルディスクリプタが`ready`状態になるまで待つ。
	`ready`... 対応するI/O操作がブロックされることなく実行可能である状態
	timeout引数に値の0timeval構造体を渡すとポーリングになる
	timeout引数にNULLを渡した時は、readyがなければ即時returnする->上の状況と同じ
	********************************************************************/
	int result = 0;
	if ((result = select(nfds, &readfds_, &writefds_, NULL, NULL)) == -1) {
		throw SysCallFailed();
	} else if (result == 0) {
		// selectがtimeoutした時
		// TODO
	}
}

void Config::accept_sockets() {
	for (size_t i=0; i<servers_.size(); ++i) {
		int sockfd = servers_[i].getSockfd();
		if (FD_ISSET(sockfd, &readfds_)) {
			clients_.push_back(Client(servers_[i]));
			Client& client = clients_.back();
			/*********************************************************************
			 int accept(int socket, struct sockaddr *restrict address,
       	  socklen_t *restrict address_len);
				保留状態の接続要求が入っているキューから先頭の接続要求を取り出し、
				接続済みソケットを新規に作成し、そのソケットを参照する新しいファイルディスクリプタを返す
				ここではこれをclientのsocketとして設定する
			**********************************************************************/
			struct sockaddr address;
			socklen_t address_len = sizeof(struct sockaddr_storage);
			int cl_sockfd = accept(servers_[i].getSockfd(), &address, &address_len);
			if (cl_sockfd == -1) {
				throw SysCallFailed();
			}
			client.setSockfd(cl_sockfd);
		}
	}
}

void Config::get_request() {
	for (size_t i=0; i<clients_.size(); i++) {
		if (FD_ISSET(clients_[i].getSockfd(), &readfds_)) {
			
		}
	}
}

void Config::event_loop() {
	this->set_select();
	this->accept_sockets();
}

void Config::create_sockets() {
	for (size_t i=0; i<servers_.size(); i++) {
		this->servers_[i].create_socket();
	}
}

void Config::close_sockets() {
	for (size_t i=0; i<servers_.size(); i++) {
		close(this->servers_[i].getSockfd());
	}
}
