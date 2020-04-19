/** ***************************************************************************
 * Various helper functions
 *
 * (c)2020 Christian Bendele
 * 
 */

#include "Helper.h"

#include <sstream>
#include <iostream>

namespace Grandma {

std::vector<std::string> Helper::vectorize_path(const std::string path) {
  std::stringstream ss;

  if(path[0] == '/') {
    ss << path.substr(1);
  } else {
    ss << path;
  }

  std::string segment;
  std::vector<std::string> vectorized;

  while(getline(ss, segment, '/')) {
    vectorized.push_back(segment);
  }
  return vectorized;
}

// TODO: this can hardly be called a real URL parser. It is very simplicistic and supports only a
// small part of RFCxxxx. Also, string operations in C++ are not my forte...
bool Helper::URL::parse_from_string(std::string s_uri) {
  std::cout << "Parsing URI " << s_uri  << std::endl;

  // parse protocol
  size_t delim = s_uri.find("://");
  if(delim == std::string::npos) return false;
  std::string proto_s = s_uri.substr(0, delim);
  std::cout << "proto_s is " << proto_s << std::endl;
  if("http" == proto_s || "HTTP" == proto_s) {
    protocol = Protocol::HTTP;
  } else if("https" == proto_s || "HTTPS" == proto_s) {
    protocol = Protocol::HTTPS;
  } else {
    return false;
  }
  s_uri = s_uri.substr(delim+3);

  // parse path
  delim = s_uri.find("/");
  if(delim == std::string::npos) {
    path = "/";
  } else {
    path = s_uri.substr(delim);
    s_uri = s_uri.substr(0, delim);
  }
  std::cout << "Path is " << path << std::endl;

  //parse port
  delim = s_uri.find(":");
  if(delim == std::string::npos) {
    port = 8080;
  } else {
    std::string s_port = s_uri.substr(delim+1);
    port = std::stoi(s_port); // FIXME: may throw exception
  } 
  std::cout << "Port is " << port << std::endl;

  // reminder is server
  server = s_uri.substr(0, delim);
  std::cout << "server is " << server << std::endl;

  return true;
}



} // namespace
