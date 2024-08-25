#include "request_handler.hpp"

RequestHandler::RequestHandler()
    : request_(NULL), response_(NULL), location_(NULL),
      rootPath_(""), relativePath_("") {}

RequestHandler::RequestHandler(HttpRequest &request, HttpResponse &response, ServerConfig &config)
    : request_(&request), response_(&response),
      rootPath_(""), relativePath_("") {
  std::vector<Location> locations = config.getLocations();
  bool location_found = false;
  for (size_t i = 0; i < locations.size(); i++) {
    // TODO: Locationのマッチング
  }
  if (!location_found) {
    // TODO: serverconfigのrootを使う
    // TODO: rootが設定されていなければ404
  }
  RequestHandler handler;
  // 暫定的にpathを設定
  rootPath_ = "/tmp";
  relativePath_ = request.getUri();
  location_ = NULL;
}

RequestHandler::RequestHandler(const RequestHandler &src)
    : request_(src.request_), response_(src.response_),
      location_(src.location_), rootPath_(src.rootPath_),
      relativePath_(src.relativePath_) {}

RequestHandler::~RequestHandler() {}

RequestHandler &RequestHandler::operator=(const RequestHandler &src) {
  if (this != &src) {
    request_ = src.request_;
    response_ = src.response_;
    location_ = src.location_;
    rootPath_ = src.rootPath_;
    relativePath_ = src.relativePath_;
  }
  return *this;
}

void RequestHandler::process() {
  switch (request_->getMethod()) {
    case GET:
      handleStaticGet();
      break;
    case POST:
      handleStaticPost();
      break;
    case DELETE:
      handleStaticDelete();
      break;
    default:
      response_->setStatus(METHOD_NOT_ALLOWED);
      break;
  }
}

void RequestHandler::handleStaticGet() {
  // rootPath_ + relativePath_ のファイルを読み、responseに書き込む
}

#include <fstream>
void RequestHandler::handleStaticPost() {
  // rootPath_ + relativePath_ にファイルを保存し、responseにステータスを書き込む
  std::string path = rootPath_ + relativePath_;
  std::ofstream ofs(path);
  if (!ofs) {
    response_->setStatus(FORBIDDEN);
    return;
  }
  ofs << request_->getBody();
  response_->setStatus(OK);
}

void RequestHandler::handleStaticDelete() { //処理順が違う可能性あり、おそらくどうでもいい
  std::string path = rootPath_ + relativePath_;
  Result<bool> is_file = filemanip::pathExists(path);
  if (!is_file.isOk()) {
    response_->setStatus(INTERNAL_SERVER_ERROR);
    return;
  }
  if (!is_file.getValue()) {
    response_->setStatus(NOT_FOUND);
    return;
  }
  Result<bool> is_dir = filemanip::isDir(path);
  if (!is_dir.isOk()) {
    response_->setStatus(INTERNAL_SERVER_ERROR);
    return;
  }
  if (is_dir.getValue()) {
    response_->setStatus(BAD_REQUEST);
    return;
  }
  if (remove(path.c_str()) != 0) {
    response_->setStatus(INTERNAL_SERVER_ERROR);
    return;
  }
  Result<bool> is_deletable = filemanip::isDeletable(path);
  if (!is_deletable.isOk()) {
    response_->setStatus(INTERNAL_SERVER_ERROR);
    return;
  }
  if (!is_deletable.getValue()) {
    response_->setStatus(FORBIDDEN);
    return;
  }
  response_->setStatus(OK);
}
