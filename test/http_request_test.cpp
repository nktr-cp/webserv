#include "http_request.hpp"
#include "http_response.hpp"
#include <iostream>
#include <string>

int main() {
    std::string raw_request = "GET / HTTP/1.1\r\n"
                              "Host: localhost:8080\r\n"
                              "User-Agent: curl/7.68.0\r\n"
                              "Accept: */*\r\n"
                              "\r\n"
                              "Hello, world!";
    std::cout << raw_request << std::endl;
    try {
        HttpRequest request(raw_request.c_str());
        std::cout << request.getMethod() << std::endl;
        std::cout << request.getUri() << std::endl;
        std::cout << request.getVersion() << std::endl;
        
        std::cout << request.getBody() << std::endl;
    }
    catch (const http::responseStatusException &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
