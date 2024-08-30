#ifndef SESSION_HPP_
#define SESSION_HPP_

#include <cstdlib>
#include <ctime>
#include <map>
#include <string>

#include "http_request.hpp"
#include "http_response.hpp"

struct Cookie {
  std::string parseCookie(const std::string& cookie_header);
  std::string createCookieHeader(const std::string& key,
                                 const std::string& value);
};

struct Session {
  std::string id;
  unsigned int numAccesses;  // prepare easy example の項目のため
  std::time_t lastAccessed;

  Session();
  Session(const std::string& sessionId);
};

class SessionManager {
 private:
  static const int kSessionAge = 60 * 60;  // 1 hour
  static const int kSessionIdLength = 32;
  std::map<std::string, Session> sessions_;

  std::string generateSessionId();

 public:
  SessionManager();

  Session* createSession();
  Session* getSession(const std::string& sessionId);
  void removeSession(const std::string& sessionId);
  void setSessionInfo(HttpRequest& request, HttpResponse& response);
};

#endif  // SESSION_HPP_
