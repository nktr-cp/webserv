#include "request_handler.hpp"

RequestHandler::RequestHandler()
    : request_(NULL),
      response_(NULL),
      location_(NULL),
      rootPath_(""),
      relativePath_("") {}

RequestHandler::RequestHandler(HttpRequest &request, HttpResponse &response,
                               ServerConfig &config)
    : request_(&request),
      response_(&response),
      rootPath_(""),
      relativePath_("") {
  std::vector<Location> locations = config.getLocations();
  size_t max_count = -1;
  Location *location = NULL;
  for (size_t i = 0; i < locations.size(); i++) {
    const std::string& path = locations[i].getRoot();
    const std::string& uri = request.getUri();
    for (size_t cur = 0; cur < path.size() && cur < uri.size(); cur++) {
      if (path[cur] != uri[cur]) {
        break;
      }
      if (path[i] == '/' && cur > max_count) {
        max_count = cur;
        location = &locations[i];
      }
    }
  }
  if (!this->location_) {
    response_->setStatus(NOT_FOUND);
  } else {
    location_ = location;
    rootPath_ = location->getRoot();
    relativePath_ = request.getUri().substr(max_count);
  }
}

RequestHandler::RequestHandler(const RequestHandler &src)
    : request_(src.request_),
      response_(src.response_),
      location_(src.location_),
      rootPath_(src.rootPath_),
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

std::string RequestHandler::generateDirectoryListing(const std::string &path) {
  DIR *dir;
  struct dirent *entry;
  struct stat file_stat;
  std::stringstream html;

  html << "<html><head><title>Index of " << relativePath_ << "</title></head>"
       << "<body><h1>Index of " << relativePath_ << "</h1><hr><pre>";

  dir = opendir(path.c_str());
  if (dir == NULL) {
    return "Error opening directory.";
  }

  while ((entry = readdir(dir)) != NULL) {
    std::string filename = entry->d_name;
    std::string fullpath = path + "/" + filename;

    if (stat(fullpath.c_str(), &file_stat) == -1) {
      continue;
    }

    html << "<a href=\"" << filename;
    if (S_ISDIR(file_stat.st_mode)) {
      html << "/";
    }
    html << "\">" << filename;
    if (S_ISDIR(file_stat.st_mode)) {
      html << "/";
    }
    html << "</a>";

    // 整列のためのpadding
    // TODO: うまく整列されてない
    int padding = 50 - filename.length();
    html << std::string(padding > 0 ? padding : 1, ' ');

    char time_str[26];
    strftime(time_str, sizeof(time_str), "%d-%b-%Y %H:%M",
             localtime(&file_stat.st_mtime));
    html << time_str << "    ";

    if (S_ISDIR(file_stat.st_mode)) {
      html << "   -";
    } else {
      html << std::setw(8) << file_stat.st_size;
    }

    html << "\n";
  }

  closedir(dir);

  html << "</pre><hr></body></html>";
  return html.str();
}

std::string RequestHandler::getMimeType(const std::string &path) {
  std::string extension = path.substr(path.find_last_of(".") + 1);
  if (extension == "html" || extension == "htm") return "text/html";
  if (extension == "css") return "text/css";
  if (extension == "js") return "application/javascript";
  if (extension == "jpg" || extension == "jpeg") return "image/jpeg";
  if (extension == "png") return "image/png";
  if (extension == "gif") return "image/gif";
  return "application/octet-stream";
}

void RequestHandler::handleStaticGet() {
  // rootPath_ + relativePath_ のファイルを読み、responseに書き込む
  std::string path = rootPath_ + relativePath_;

  Result<bool> fileExists = filemanip::pathExists(path);
  if (!fileExists.isOk() || !fileExists.getValue()) {
    response_->setStatus(NOT_FOUND);
    return;
  }

  Result<bool> isDir = filemanip::isDir(path);
  if (!isDir.isOk()) {
    response_->setStatus(INTERNAL_SERVER_ERROR);
    return;
  }
  if (isDir.getValue()) {
    // MEMO:
    // indexファイルは複数指定できるので、ここはstd::vector<std::string>のはず
    std::string indexPath = path + "/" + location_.getIndex();
    Result<bool> indexExists = filemanip::pathExists(indexPath);
    // indexファイルが存在するか？
    if (indexExists.isOk() && indexExists.getValue()) {
      path = indexPath;
    } else {
      if (location_.isAutoIndex()) {
        std::string listing = generateDirectoryListing(path);
        response_->setBody(listing);
        response_->setHeader("Content-Type", "text/html");
        response_->setStatus(OK);
        return;
      } else {
        response_->setStatus(FORBIDDEN);
        return;
      }
    }
  }

  std::ifstream ifs(path.c_str());
  if (!ifs) {
    response_->setStatus(FORBIDDEN);
    return;
  }

  std::stringstream buffer;
  buffer << ifs.rdbuf();

  if (ifs.fail() && !ifs.eof()) {
    response_->setStatus(INTERNAL_SERVER_ERROR);
    return;
  }

  std::string mimeType = getMimeType(path);

  response_->setHeader("Content-Type", mimeType);
  response_->setBody(buffer.str());
  response_->setStatus(OK);
}

void RequestHandler::handleStaticPost() {
  // rootPath_ + relativePath_
  // にファイルを保存し、responseにステータスを書き込む
  std::string path = rootPath_ + relativePath_;
  std::ofstream ofs(path.c_str());
  if (!ofs) {
    response_->setStatus(FORBIDDEN);
    return;
  }
  ofs << request_->getBody();
  response_->setStatus(OK);
}

void RequestHandler::
  handleStaticDelete() {  // 処理順が違う可能性あり、おそらくどうでもいい
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
