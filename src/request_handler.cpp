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
      config_(&config),
      rootPath_(""),
      relativePath_("")
{
  if (static_cast<size_t>(config.getMaxBodySize()) < request.getBody().size())
  {
    response_->setStatus(PAYLOAD_TOO_LARGE);
    return;
  }
  const std::vector<Location> &locations = config.getLocations();
  int max_count = -1;
  const Location *location = NULL;
  std::string uri = request.getUri();
  if (uri[uri.size() - 1] != '/')
  {
    uri += "/";
  }
  for (size_t i = 0; i < locations.size(); i++)
  {
    const std::string &path = locations[i].getName();
    for (size_t cur = 0; cur < path.size() && cur < uri.size(); cur++)
    {
      if (path[cur] != uri[cur])
      {
        break;
      }
      if (path[cur] == '/' && (int)cur > max_count)
      {
        max_count = cur;
        location = &locations[i];
      }
    }
  }
  if (location == NULL)
  {
    response_->setStatus(NOT_FOUND);
    return;
  }
  else
  {
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

RequestHandler &RequestHandler::operator=(const RequestHandler &src)
{
  if (this != &src)
  {
    request_ = src.request_;
    response_ = src.response_;
    location_ = src.location_;
    rootPath_ = src.rootPath_;
    relativePath_ = src.relativePath_;
  }
  return *this;
}
void RequestHandler::process()
{
  if (location_->isRedirect())
  {
    // std::cerr << "Handling:\tredirect" << std::endl;
    response_->setStatus(FOUND);
    response_->setHeader("Location", location_->getRedirect());
    return;
  }
  if (response_->getStatus() != OK)
  {
    return;
  }
  // std::cerr << "Location:\t" << location_->getName() << std::endl;
  if (location_->isCgi())
  {
    // std::cerr << "Handling:\tCGI "
    // << http::methodToString(request_->getMethod()) << std::endl;
    handleCGIRequest();
    return;
  }
  // std::cerr << "Handling:\t" << http::methodToString(request_->getMethod())
  // << std::endl;
  if (!location_->isMethodAllowed(request_->getMethod()))
  {
    response_->setStatus(METHOD_NOT_ALLOWED);
    return;
  }
  switch (request_->getMethod())
  {
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
  std::string errorpage = config_->getErrorPage(response_->getStatus());
  if (!errorpage.empty())
  {
    // std::cerr << response_->getStatus() << " error:\tredirecting" <<
    // std::endl;
    response_->setStatus(FOUND);
    response_->setHeader("Location", errorpage);
  }
  try
  {
    std::string error = request_->getQuery("error");
    if (!error.empty())
    {
      const unsigned int range[2] = {100, 599};
      response_->setStatus(static_cast<HttpStatus>(ft::stoui(error, range)));
    }
  }
  catch (std::exception &e)
  {
  } // ignore
  // std::cerr << "Status:\t\t" << response_->getStatus() << std::endl;
}

FileEntry::FileEntry(const std::string &n, const std::string &m, long s,
                     bool d)
    : name(n), modTime(m), size(s), isDirectory(d) {}

std::string RequestHandler::generateDirectoryListing(const std::string &path)
{
  DIR *dir;
  struct dirent *entry;
  struct stat file_stat;
  std::stringstream html;
  std::vector<FileEntry> entries;

  dir = opendir(path.c_str());
  if (dir == NULL)
  {
    return "Error opening directory.";
  }

  while ((entry = readdir(dir)) != NULL)
  {
    std::string filename = entry->d_name;
    std::string fullpath = path + "/" + filename;

    if (filename == "." || filename == "..")
    {
      continue;
    }

    if (stat(fullpath.c_str(), &file_stat) == -1)
    {
      continue;
    }

    char time_str[26];
    strftime(time_str, sizeof(time_str), "%d-%b-%Y %H:%M",
             localtime(&file_stat.st_mtime));

    entries.push_back(
        FileEntry(filename, std::string(time_str),
                  S_ISDIR(file_stat.st_mode) ? -1 : file_stat.st_size,
                  S_ISDIR(file_stat.st_mode)));
  }

  closedir(dir);

  // sort entries based on their names
  for (size_t i = 0; i + 1 < entries.size(); i++)
  {
    for (size_t j = i + 1; j < entries.size(); j++)
    {
      if (entries[i].name > entries[j].name)
      {
        std::swap(entries[i], entries[j]);
      }
    }
  }

  html
      << "<!DOCTYPE html>\n"
      << "<html lang=\"en\">\n"
      << "<head>\n"
      << "    <meta charset=\"UTF-8\">\n"
      << "    <meta name=\"viewport\" content=\"width=device-width, "
         "initial-scale=1.0\">\n"
      << "    <title>Index of " << relativePath_ << "</title>\n"
      << "    <style>\n"
      << "        body { font-family: Arial, sans-serif; margin: 20px; }\n"
      << "        h1 { border-bottom: 1px solid #ccc; padding-bottom: 10px; }\n"
      << "        table { border-collapse: collapse; width: 100%; }\n"
      << "        th, td { text-align: left; padding: 8px; }\n"
      << "        tr:nth-child(even) { background-color: #f2f2f2; }\n"
      << "    </style>\n"
      << "</head>\n"
      << "<body>\n"
      << "    <h1>Index of " << relativePath_ << "</h1>\n"
      << "    <table>\n"
      << "        <tr>\n"
      << "            <th>Name</th>\n"
      << "            <th>Last modified</th>\n"
      << "            <th>Size</th>\n"
      << "        </tr>\n";

  html << "        <tr>\n"
       << "            <td><a href=\"../\">../</a></td>\n"
       << "            <td></td>\n"
       << "            <td></td>\n"
       << "        </tr>\n";

  for (std::vector<FileEntry>::const_iterator it = entries.begin();
       it != entries.end(); ++it)
  {
    html << "        <tr>\n"
         << "            <td><a href=\"" << it->name
         << (it->isDirectory ? "/" : "") << "\">" << it->name
         << (it->isDirectory ? "/" : "") << "</a></td>\n"
         << "            <td>" << it->modTime << "</td>\n"
         << "            <td>" << (it->isDirectory ? "-" : ft::uitost(it->size))
         << "</td>\n"
         << "        </tr>\n";
  }

  html << "    </table>\n"
       << "</body>\n"
       << "</html>";

  return html.str();
}

std::string RequestHandler::getMimeType(const std::string &path)
{
  std::string extension = path.substr(path.find_last_of(".") + 1);
  if (extension == "html" || extension == "htm")
    return "text/html";
  if (extension == "css")
    return "text/css";
  if (extension == "js")
    return "application/javascript";
  if (extension == "jpg" || extension == "jpeg")
    return "image/jpeg";
  if (extension == "png")
    return "image/png";
  if (extension == "gif")
    return "image/gif";
  return "application/octet-stream";
}

void RequestHandler::handleStaticGet()
{
  // rootPath_ + relativePath_ のファイルを読み、responseに書き込む
  std::string path = rootPath_ + relativePath_;

  Result<bool> fileExists = filemanip::pathExists(path);
  if (!fileExists.isOk() || !fileExists.getValue())
  {
    response_->setStatus(NOT_FOUND);
    return;
  }

  Result<bool> isDir = filemanip::isDir(path);
  if (!isDir.isOk())
  {
    response_->setStatus(INTERNAL_SERVER_ERROR);
    return;
  }
  if (isDir.getValue())
  {
    // Use vector of index files
    const std::vector<std::string> &indexFiles = location_->getIndex();
    bool indexFound = false;
    for (std::vector<std::string>::const_iterator it = indexFiles.begin();
         it != indexFiles.end(); ++it)
    {
      std::string indexPath = path + "/" + *it;
      Result<bool> indexExists = filemanip::pathExists(indexPath);
      if (indexExists.isOk() && indexExists.getValue())
      {
        path = indexPath;
        indexFound = true;
        break;
      }
    }
    if (!indexFound)
    {
      if (location_->isAutoIndex())
      {
        std::string listing = generateDirectoryListing(path);
        response_->setBody(listing);
        response_->setStatus(OK);
        return;
      }
      else
      {
        if (!indexFiles.empty())
          response_->setStatus(NOT_FOUND);
        else
          response_->setStatus(FORBIDDEN);
        return;
      }
    }
  }

  std::ifstream ifs(path.c_str());
  if (!ifs)
  {
    response_->setStatus(FORBIDDEN);
    return;
  }

  std::stringstream buffer;
  buffer << ifs.rdbuf();

  if (ifs.fail() && !ifs.eof())
  {
    response_->setStatus(INTERNAL_SERVER_ERROR);
    return;
  }
  std::string mimeType = getMimeType(path);

  response_->setHeader("Content-Type", mimeType);
  response_->setBody(buffer.str());
  response_->setStatus(OK);
}

void RequestHandler::handleStaticPost()
{
  // rootPath_ + relativePath_
  // にファイルを保存し、responseにステータスを書き込む
  std::string path = rootPath_ + relativePath_;
  std::ofstream ofs(path.c_str());
  if (!ofs)
  {
    response_->setStatus(FORBIDDEN);
    return;
  }
  ofs << request_->getBody();
  response_->setStatus(OK);
}

void RequestHandler::
    handleStaticDelete()
{ // 処理順が違う可能性あり、おそらくどうでもいい
  std::string path = rootPath_ + relativePath_;

  Result<bool> is_file = filemanip::pathExists(path);
  if (!is_file.isOk())
  {
    response_->setStatus(INTERNAL_SERVER_ERROR);
    return;
  }
  if (!is_file.getValue())
  {
    response_->setStatus(NOT_FOUND);
    return;
  }

  Result<bool> is_dir = filemanip::isDir(path);
  if (!is_dir.isOk())
  {
    response_->setStatus(INTERNAL_SERVER_ERROR);
    return;
  }
  if (is_dir.getValue())
  {
    response_->setStatus(BAD_REQUEST);
    return;
  }

  Result<bool> is_deletable = filemanip::isDeletable(path);
  if (!is_deletable.isOk())
  {
    response_->setStatus(INTERNAL_SERVER_ERROR);
    return;
  }
  if (!is_deletable.getValue())
  {
    response_->setStatus(FORBIDDEN);
    return;
  }

  if (remove(path.c_str()) != 0)
  {
    response_->setStatus(INTERNAL_SERVER_ERROR);
    return;
  }

  response_->setStatus(OK);
}

void RequestHandler::handleCGIRequest()
{//WIP
  CgiMaster cgi(request_, response_, location_);
  try
  {
    cgi.execute();
  }
  catch (SysCallFailed &e)
  {
    response_->setStatus(INTERNAL_SERVER_ERROR);
    return;
  }
  catch (http::responseStatusException &e)
  {
    response_->setStatus(e.getStatus());
    return;
  }
}