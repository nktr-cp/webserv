#include "CGIHandler.hpp"

CGIHandler::CGIHandler(std::string script_path)
: script_path_(script_path) {}

// 多分ここの設定が足りてない
static char** setup_env(const Request& request) {
	std::vector<std::string> env_string;
	char** envp = new char*[sizeof(char*) * (request.get_headers().size() + 3)];

	env_string.push_back("REQUEST_METHOD=" + request.get_method());
	env_string.push_back("SCRIPT_NAME=" + request.get_path());
	env_string.push_back("CONTENT_LENGTH=" + std::to_string(request.get_body().size()));

	dict headers = request.get_headers();
	for (dict::iterator it = headers.begin(); it != headers.end(); it++) {
		env_string.push_back("HTTP_" + it->first + "=" + it->second);
	}

	size_t i = 0;
	for (; i < env_string.size(); i++) {
		envp[i] = strdup(env_string[i].c_str());
		if (envp[i] == NULL) {
			throw std::runtime_error("strdup failed");
		}
	}
	envp[i] = NULL;

	return envp;
}

static dict parse_headers(const std::string& headers_string) {
	dict headers;
	std::istringstream headers_stream(headers_string);
	std::string line;

	while (std::getline(headers_stream, line) && !line.empty()) {
		size_t colon_pos = line.find(':');
		if (colon_pos != std::string::npos) {
			headers.insert(std::make_pair(line.substr(0, colon_pos), line.substr(colon_pos + 2)));
		}
	}
	return headers;
}

Response CGIHandler::execute(Request& request) {
	// set up environment variables
	char** envp = setup_env(request);
	std::cout << "DEBUG: content of envp\n";
	for (size_t i = 0; envp[i] != NULL; i++) {
		std::cout << envp[i] << std::endl;
	}

	// create pipes for IPC
	int pipe_in[2], pipe_out[2];
	pipe(pipe_in);
	pipe(pipe_out);

	pid_t pid = fork();
	if (pid == 0) {
		// child process
		dup2(pipe_in[0], STDIN_FILENO), dup2(pipe_out[1], STDOUT_FILENO);
		close(pipe_in[1]), close(pipe_out[0]);

		char* argv[] = { const_cast<char*> (script_path_.c_str()), NULL };
		execve(script_path_.c_str(), argv, envp);
		// reach if execve fails
		perror("execve"); // Exec format error
		exit(1);
	} else {
		// parent process
		close(pipe_in[0]), close(pipe_out[1]);

		write(pipe_in[1], request.get_body().c_str(), request.get_body().size());
		close(pipe_in[1]);

		// capture the output
		char buffer[4096];
		std::string output;
		ssize_t bytes_read;
		while ((bytes_read = read(pipe_out[0], buffer, sizeof(buffer))) > 0) {
			output.append(buffer, bytes_read);
		}
		std::cout << "DEBUG: content of output\n" << output << std::endl;
		close(pipe_out[0]);
		waitpid(pid, NULL, 0);

		std::istringstream response_stream(output);
		std::string status_line;
		std::getline(response_stream, status_line);

		int status_code = 200;
		if (status_line.find("Status: ") != std::string::npos) {
			status_code = std::stoi(status_line.substr(7));
		}

		std::string headers_string;
		std::string line;
		while (std::getline(response_stream, line) && !line.empty()) {
			headers_string += line + "\n";
		}

		std::string body;
		while (std::getline(response_stream, line)) {
			body += line + "\n";
		}

		return Response(status_code, parse_headers(headers_string), body);
	}
}
