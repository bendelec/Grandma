#include "Session.h"

#include <iostream>
#include <nlohmann/json.hpp>

#include "base64.h"

namespace Grandma {
  
  using namespace nlohmann;

  // httplib::Client http_client("10.223.27.113", 9988);
  // httplib::SSLClient http_client("10.223.27.113", 9988);
  httplib::Client http_client("localhost", 9988);

  Session::Session(MOTree &motree) : motree(motree){}

  std::string Session::send_P1(std::string p1_json) {
    // http(s) send P1
    httplib::Headers headers = {
      {"OMADM-DevID", "PlanB"},
      {"Accept", "application/vnd.oma.dm.request+json"}
    };
    


    // temporarily use base64 encoding for testing against TM server
    std::string p1_base64 = base64_encode(reinterpret_cast<const unsigned char *>(p1_json.c_str()), p1_json.length());
    // auto res = http_client.Post("path", headers, p1_base64, "application/vnd.oma.dm.initiation+json");
    auto res = http_client.Post("/path", headers, p1_json, "application/vnd.oma.dm.initiation+json");
    
    if(res) {
      std::cerr << "http result is: " << res->status << std::endl;
      std::string p2_body = res->body;
      std::cout << "Server response is:" << std::endl << p2_body << std::endl;
      return p2_body;
    } else {
      std::cerr << "ERROR: connection to server failed when trying to send P1" << std::endl;
      return "";
    }

    // receive P2?
    // std::string p2_body = base64_decode(res->body);

  }

  // TODO: this can hardly be called a real URL parser. It is very simplicistic and supports only a
  // small part of RFCxxxx. Also, string operations in C++ are not my forte...
  bool Session::URL::parse_from_string(std::string s_uri) {
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

  void Session::do_hget(json command) {
    (void)command;
    std::string clientURI = command[2];	// TODO: this is actually optional. We need to support client side decision on where to put the downloaded data
    
    std::cout << "IN do_hget. ServerURI = " << command[1] << " - ClientURI = " << clientURI << std::endl;

    URL serverURL;
    serverURL.parse_from_string(command[1]);
    
    std::shared_ptr<httplib::Response> res;
    if(serverURL.protocol == URL::Protocol::HTTPS) {
      std::cerr << "Opening HTTPS connection" << std::endl;
      res = httplib::SSLClient(serverURL.server, serverURL.port).Get(serverURL.path.c_str());
    } else {
      std::cerr << "Opening HTTP connection" << std::endl;
      res = httplib::Client(serverURL.server, serverURL.port).Get(serverURL.path.c_str());
    }

    std::cout << "Server response: " << res->status << ": " << std::endl << res->body << std::endl;

    motree.node_set(clientURI, json(res->body));

    // TODO: parse serverURI for http vs https instead of assuming https
    //httplib::SSLClient command_client();
  }

  bool Session::parse_P2(std::string p2_json) {
    bool session_continue = true;

    std::cout << "Parsing package 2:" << std::endl;
    json commands;
    try {
      commands = json::parse(p2_json)["CMD"];
    } catch (const nlohmann::detail::parse_error &err) {
      std::cout << "ERROR parsing P2 received from server. Ending session." << std::endl;
      session_continue = false;
    }
    for(auto &command : commands) {
      std::cout << command[0] << std::endl;
      if(command[0] == "END") {
	std::cout << "Received END command." << std::endl;
	session_continue = false;
	continue;
      }
      if(command[0] == "CONT") {
	std::cout << "Received CONT command." << std::endl;
	
	continue;
      }
      if(command[0] == "HGET") {
	std::cout << "Received HGET command." << std::endl;
	do_hget(command);
	continue;
      }
      if(command[0] == "HPUT") {
	std::cout << "Received HPUT command." << std::endl;
	continue;
      }
      if(command[0] == "HPOST") {
	std::cout << "Received HPOST command." << std::endl;
	continue;
      }
      if(command[0] == "DELETE") {
	std::cout << "Received DELETE command." << std::endl;
	continue;
      }
      if(command[0] == "EXEC") {
	std::cout << "Received EXEC command." << std::endl;
	continue;
      }
      if(command[0] == "GET") {
	std::cout << "Received HET command." << std::endl;
	continue;
      }
      if(command[0] == "SHOW") {
	std::cout << "Received SHOW command." << std::endl;
	continue;
      }
      if(command[0] == "DEFAULT") {
	std::cout << "Received DEFAULT command." << std::endl;
	continue;
      }
      if(command[0] == "SUB") {
	std::cout << "Received SUB command." << std::endl;
	continue;
      }
      if(command[0] == "UNSUB") {
	std::cout << "Received UNSUB command." << std::endl;
	continue;
      }
      std::cout << "WARNING: Received unknown command " << command[0] << ". Ignoring." << std::endl;
    }
    return session_continue;
  }

  std::string Session::send_P3(std::string p3_json) {
    (void)p3_json;
    return "{\"CMD\": [ [ \"END\" ] ] }";
  }

} // namespace
