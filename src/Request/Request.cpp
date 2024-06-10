
#include "request.hpp"

/* sample request (nginx)
GET / HTTP/1.1
Accept:
text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,/*//*;q=0.8
Accept-Encoding: gzip, deflate, br, zstd
Accept-Language: en-US,en
Cache-Control: max-age=0
Connection: keep-alive
Host: localhost:8080
If-Modified-Since: Tue, 16 Apr 2024 14:37:23 GMT
If-None-Match: "661e8d23-267"
Sec-Fetch-Dest: document
Sec-Fetch-Mode: navigate
Sec-Fetch-Site: none
Sec-Fetch-User: ?1
Sec-GPC: 1
Upgrade-Insecure-Requests: 1
User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/125.0.0.0 Safari/537.36
sec-ch-ua: "Brave";v="125", "Chromium";v="125", "Not.A/Brand";v="24"
sec-ch-ua-mobile: ?0
sec-ch-ua-platform: "macOS"
*/

Request::Request(std::string row_request) {
  std::istringstream iss(row_request);
  std::string line;

  // omit error handling so far
  if (std::getline(iss, line)) {
    char delim;  // this should be '/'
    std::istringstream request_line_stream(line);
    request_line_stream >> method_ >> delim >> path_;
  } else {
    throw std::runtime_error("Empty request");
  }

  // parse headers
  while (std::getline(iss, line) && !line.empty()) {
    size_t pos = line.find(':');
    if (pos != std::string::npos) {
      std::string key = line.substr(0, pos);
      std::string value = line.substr(pos + 2);
      headers_[key] = value;
    }
  }

  // parse body
  if (headers_.find("Content-Length") != headers_.end()) {
    // ft::の関数使えなかったので(throwされる)、差し向きstd::stoi
    int content_length = std::stoi(headers_["Content-Length"]);
    body_.resize(content_length);
    iss.read(&body_[0], content_length);
  } else {
    // throw std::runtime_error("Content-Length header not found");
    body_ = "";
  }
}

Request::Request(std::string raw_request) { parse(raw_request); }

std::string Request::get_method() const { return method_; }

std::string Request::get_path() const { return path_; }

dict Request::get_headers() const { return headers_; }

std::string Request::get_body() const { return body_; }
