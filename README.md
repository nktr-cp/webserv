# 🌐 Webserv

![C++](https://img.shields.io/badge/C++-98-blue.svg?style=for-the-badge&logo=c%2B%2B)
![HTTP](https://img.shields.io/badge/HTTP-1.1-orange.svg?style=for-the-badge&logo=http)

A lightweight, high-performance HTTP server implemented in C++98. This project aims to deepen understanding of HTTP protocols, network programming, and server architecture.

## 🚀 Features

- **HTTP/1.1 Compliant**: Implements core features of the HTTP/1.1 protocol
- **Multi-port Listening**: Can listen on multiple ports as specified in the configuration
- **Static File Serving**: Efficiently serves static websites
- **Dynamic Content**: Supports CGI for dynamic content generation
- **File Uploads**: Handles file uploads from clients
- **Custom Error Pages**: Configurable custom error pages
- **Request Methods**: Supports GET, POST, and DELETE methods
- **Non-blocking I/O**: Uses epoll() and kqueue() for efficient I/O operations

## 🛠 Setup

### Prerequisites

- C++ compiler with C++98 support
- Make

### Compilation

Clone the repository and compile the project:

```bash
git clone https://github.com/nktr-cp/webserv.git
cd webserv
make
```

## 🖥 Usage

Run the server with a configuration file:

```bash
./webserv [path_to_config_file]
```

If no configuration file is specified, it will use a default path (`conf/default.conf`).

## 📁 Configuration

The server is configured using a configuration file. Here's a basic example:

```nginx
server {
	listen 8080 ;
	server_name default_server;
	errorpage 301 https://42tokyo.jp;
	
	location / {
		method GET;
		autoindex on;
		index index.html nginx.html;
		root ./;
	}

	location /html/ {
		method GET;
		root /html/;
		autoindex on;
		index index.html;
	}
}
```

Refer to the documentation for more detailed configuration options.

## 🧪 Testing

We recommend testing the server using various methods:

1. Web browsers
2. Command-line tools like `curl`
3. Custom test scripts (e.g., in Python or Go)
4. The provided tester tool

Ensure to test different scenarios, including concurrent connections and edge cases.

## 📚 Resources

- [HTTP/1.1 RFC](https://tools.ietf.org/html/rfc2616)
- [NGINX documentation](https://nginx.org/en/docs/) (for configuration inspiration)

---

## 📊 Architecture

### Class Diagram

```mermaid
classDiagram

class Config {
  -vector~ServerConfig~ serverConfigs_
  +Config(string filename)
  +vector~ServerConfig~ getServerConfigs() const
  -void parse()
}

class ServerConfig {
  -vector~Location~ locations_
  -string host_
  -string port_
  +void setHost(const string &)
  +void setPort(const string &)
  +const vector~Location~ &getLocations() const
  +const string &getHost() const
  +const string &getPort() const
}

class Location {
  -string name_
  -string root_
  -bool autoindex_
  +void setName(const string&)
  +void setRoot(const string&)
  +void setAutoindex(bool)
  +const string& getName() const
  +const string& getRoot() const
  +bool isAutoIndex() const
}

class HttpRequest {
  -HttpMethod method_
  -string uri_
  -dict headers_
  +HttpRequest(const char* raw_request)
  +HttpMethod getMethod() const
  +const string& getUri() const
  +const dict& getHeader() const
}

class HttpResponse {
  -HttpStatus status_
  -dict headers_
  +HttpResponse()
  +void setStatus(HttpStatus status)
  +void setHeader(const string &key, const string &value)
  +string encode() const
}

class RequestHandler {
  -HttpRequest* request_
  -HttpResponse* response_
  -const Location* location_
  +RequestHandler(HttpRequest &request, HttpResponse &response, ServerConfig &config)
  +void process()
  -void handleStaticGet()
  -void handleStaticPost()
  -void handleStaticDelete()
}

class Server {
  -vector~ServerConfig~ config_
  -SessionManager sessionManager_
  -int server_fd_
  +Server(vector~ServerConfig~ config)
  +void createSocket()
  +void handleRequest(HttpRequest& request, HttpResponse& response)
}

class Webserv {
  -vector~Server~ servers_
  +Webserv(const string &configFile)
  +void run()
}

class SessionManager {
  -map~string, Session~ sessions_
  +SessionManager()
  +Session* createSession()
  +Session* getSession(const string& sessionId)
  +void removeSession(const string& sessionId)
  +void setSessionInfo(HttpRequest& request, HttpResponse& response)
}

class Session {
  -string id
  -unsigned int numAccesses
  -time_t lastAccessed
  +Session(const string& sessionId)
}

Config --> ServerConfig
ServerConfig --> Location
RequestHandler --> HttpRequest
RequestHandler --> HttpResponse
RequestHandler --> Location
Webserv --> Server
Server --> ServerConfig
Server --> SessionManager
SessionManager --> Session

```

### Sequence Diagram
sequenceDiagram
```mermaid
sequenceDiagram
    participant Client
    participant Webserv
    participant Server
    participant SessionManager
    participant RequestHandler
    participant HttpRequest
    participant HttpResponse

    Webserv ->> Server: Create Server Sockets
    Server ->> Server: createSocket()
    
    loop Wait for Client Request
        Client ->> Server: Send HTTP Request
        Server ->> SessionManager: Check/Create Session
        Server ->> RequestHandler: Handle Request

        RequestHandler ->> HttpRequest: Parse Request
        HttpRequest ->> RequestHandler: Return Parsed Data

        RequestHandler ->> HttpResponse: Generate Response
        HttpResponse ->> RequestHandler: Return Response

        RequestHandler ->> Server: Send Response
        Server ->> Client: Return HTTP Response
    end

```

## 👥 Team

This project was developed collaboratively by:

- [Wolframike](https://github.com/Wolframike)
- [oda251](https://github.com/oda251)
- [nktr-cp](https://github.com/nktr-cp)