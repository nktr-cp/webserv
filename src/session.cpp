#include "session.hpp"

std::string Cookie::parseCookie(const std::string& raw) {
  // MEMO: keyに対して柔軟にする必要がありそう
  std::string key = "sid";
  std::string value = "";
  size_t pos = raw.find(key);
  if (pos != std::string::npos) {
    pos += key.size() + 1;
    size_t end = raw.find(";", pos);
    value = raw.substr(pos, end - pos);
  }
  return value;
}

std::string Cookie::createCookieHeader(const std::string& key,
                                       const std::string& value) {
  return key + "=" + value + "; Path=/; HttpOnly" + "; Max-Age=3600";  // 1 hour
}

Session::Session() : id(""), numAccesses(0), lastAccessed(time(NULL)) {}

Session::Session(const std::string& sessionId)
    : id(sessionId), numAccesses(0), lastAccessed(time(NULL)) {}

SessionManager::SessionManager() {
  std::srand(static_cast<unsigned int>(time(NULL)));
}

std::string SessionManager::generateSessionId() {
  const char sessionIdChars[] =
      "0123456789"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz";

  std::string id;
  for (int i = 0; i < kSessionIdLength; ++i) {
    id += sessionIdChars[std::rand() % (sizeof(sessionIdChars) - 1)];
  }
  return id;
}

Session* SessionManager::createSession() {
  std::string sessionId = generateSessionId();

  Session newSession = Session(sessionId);

  sessions_.insert(std::make_pair(sessionId, newSession));
  return &sessions_[sessionId];
}

Session* SessionManager::getSession(const std::string& sessionId) {
  std::map<std::string, Session>::iterator it = sessions_.find(sessionId);
  if (it != sessions_.end()) {
    // if the session is exprired, remove it here
    std::time_t now = time(NULL);
    if (now - it->second.lastAccessed > kSessionAge) {
      sessions_.erase(it);
      return NULL;
    }
    it->second.lastAccessed = now;
    return &it->second;
  }
  return NULL;
}

void SessionManager::removeSession(const std::string& sessionId) {
  sessions_.erase(sessionId);
}

void SessionManager::setSessionInfo(HttpRequest& request,
                                    HttpResponse& response) {
  Cookie cookie;
  dict headers = request.getHeader();
  std::string sessionId = cookie.parseCookie(headers["Cookie"]);
  Session* session = getSession(sessionId);
  if (session == NULL) {
    session = createSession();
    response.setHeader("Set-Cookie",
                       cookie.createCookieHeader("sid", session->id));
  } else {
    session->numAccesses++;
    std::cout << "[Session management Test]: The number of accesses is "
              << session->numAccesses << " for " << session->id << std::endl;
  }
}
