#include "Config.hpp"

signed main(int argc, char **argv) {
	if (argc != 2) {
		std::cerr << "Usage: ./webserv <filename>" << std::endl;
		return 1;
	}

	try {
		Config config(argv[1]);
	} catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}
}

__attribute__((destructor))
static void	leaks(void)
{
	system("leaks -q webserv");
}
