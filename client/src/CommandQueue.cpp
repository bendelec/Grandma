#include "CommandQueue.h"

#include <iostream>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>

#include <nlohmann/json.hpp>

#include "Helper.h"

namespace Grandma {

using namespace nlohmann;
  
CommandQueue::CommandQueue(MOTree &motree) : motree(motree) {}

void CommandQueue::push_command(Command command) {
  commands.push_back(command);
}

CommandQueue::Status::Status(unsigned code) : code(code) {}

void CommandQueue::do_commands() {
  while(!commands.empty()) {
    auto command = commands.front();
    switch(command.type) {
      case CommandType::HGET:
        do_hget(command.parameter);
        break;
      default:
        responses.push_back(Status(501));
        break;
    }

    commands.pop_front();
  }
}

  void CommandQueue::do_hget(std::vector<std::string> params) {
    Helper::URL serverURL;
    if(!serverURL.parse_from_string(params[0])) {
      std::cerr << "ERROR: could not parse server URI in HGET command" << std::endl;
      return;
    };
    
    std::string clientURI = params[1];	// TODO: this is actually optional. We need to support client side decision on where to put the downloaded data
    
    std::cout << "IN do_hget. ServerURI = " << params[0] << " - ClientURI = " << clientURI << std::endl;

    std::string rbody;
    httplib::Result res;

    if(serverURL.protocol == Helper::URL::Protocol::HTTPS) {
      httplib::SSLClient cli(serverURL.server, serverURL.port);
      res = cli.Get(serverURL.path.c_str());
    } else if(serverURL.protocol == Helper::URL::Protocol::HTTP) {
      httplib::Client cli(serverURL.server, serverURL.port);
      res = cli.Get(serverURL.path.c_str());
    } else {
      std::cerr << "ERROR: unsupported protocol in HGET command" << std::endl;
      return;
    }

    if(res) {
      std::cerr << "http result is: " << res->status << std::endl;
      rbody = res->body;
    } else {
      std::cerr << "ERROR: connection to server failed when trying to send HGET command" << std::endl;
      return;
    }

    std::cout << "Server response is:" << std::endl << rbody << std::endl;
      
    motree.node_set(clientURI, json(rbody));
    responses.push_back(Status(200));

  }

  json CommandQueue::p3_SC_json() {
    json status;

    while(!responses.empty()) {
      auto response = responses.front();
      json jresponse;
      jresponse["sc"] = response.code;
      for(auto uri : response.URI) {
        jresponse["URI"].push_back(uri);
      }
      status.push_back(jresponse);
      responses.pop_front();
    }
    return status;  
  }

} // namespace
