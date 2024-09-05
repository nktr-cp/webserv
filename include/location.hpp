#ifndef LOCATION_HPP_
#define LOCATION_HPP_

#include "typedefs.hpp"

class Location {
 private:
  // parsed
  short methods_;
  std::string name_;
  std::string root_;
  bool autoindex_;
  std::vector<std::string> index_;
  std::vector<std::string> extensions_;
  std::string upoadPath_;
  std::string redirect_;
  std::string cgiPath_;

 public:
  // setter, getter
  Location();
  void addMethod(const HttpMethod);
  void setName(const std::string&);
  void setRoot(const std::string&);
  void setAutoindex(bool);
  void addIndex(const std::string&);
  void addExtension(const std::string&);
  void setUploadPath(const std::string&);
  void setRedirect(const std::string&);
  void setCgiPath(const std::string&);

  short getMethods() const;
  const std::string& getName() const;
  const std::string& getRoot() const;
  bool isAutoIndex() const;
  const std::vector<std::string>& getIndex() const;
  const std::vector<std::string>& getExtensions() const;
  const std::string& getUploadPath() const;
  const std::string& getRedirect() const;
  const std::string& getCgiPath() const;
  bool isCgi() const;
  bool isRedirect() const;
  // debug
  void print();
};

#endif  // LOCATION_HPP_