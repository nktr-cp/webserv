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
    case 1:  // HttpMethod::GET
      handleStaticGet();
      break;
    case 2:  // HttpMethod::POST
      handleStaticPost();
      break;
    case 4:  // HttpMethod::DELETE
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

void RequestHandler::handleStaticPost() {
  // rootPath_ + relativePath_ にファイルを保存し、responseにステータスを書き込む
}

void RequestHandler::handleStaticDelete() {
  // rootPath_ + relativePath_ のファイルを削除し、responseにステータスを書き込む
}
