#include "Session.h"

#include <iostream>
// #include <nlohmann/json.hpp>

#define CPPHTTPLIB_OPENSSL_SUPPORT

#include <httplib.h>
// httplib includes some arpa stuff, which defines DELETE as a macro, but we use it as a enum value for OMADM commands
#undef DELETE


#include "CommandQueue.h"

#include "base64.h"

namespace Grandma {
  
  using namespace nlohmann;

  // httplib::Client http_client("10.223.27.113", 9988);
  // httplib::SSLClient http_client("10.223.27.113", 9988);
  httplib::Client http_client("localhost", 9988);

  Session::Session(MOTree &motree, CommandQueue &command_queue) : motree(motree), command_queue(command_queue) {}

  std::string Session::send_P1(std::string p1_json) {
    // http(s) send P1
    httplib::Headers headers = {
      {"OMADM-DevID", "PlanB"},
      {"Accept", "application/vnd.oma.dm.request+json"}
    };
    
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

  bool Session::parse_P2(std::string p2_json) {
    bool session_continue = true;

    std::cout << "Parsing package 2:" << std::endl;
    json jcommands;
    try {
      jcommands = json::parse(p2_json)["CMD"];
    } catch (const nlohmann::detail::parse_error &err) {
      std::cout << "ERROR parsing P2 received from server. Ending session." << std::endl;
      session_continue = false;
    }
    for(auto &jcommand : jcommands) {
      std::cout << "Next Command is " << jcommand[0] << std::endl;
      if(jcommand[0] == "END") {
	      std::cout << "Received END command." << std::endl;
	      session_continue = false;
	      continue;
      }

      CommandQueue::Command command; // TODO presize
      for(size_t i = 1; i < jcommand.size(); ++i) {
        std::cout << "Parameter " << i - 1 << " is: " << jcommand[i] << std::endl;
        command.parameter.push_back(jcommand[i]);
      }
      if(jcommand[0] == "CONT") {
        command.type = CommandQueue::CommandType::CONT;
        command_queue.push_command(command);
	      continue;
      }
      if(jcommand[0] == "HGET") {
        command.type = CommandQueue::CommandType::HGET;
        command_queue.push_command(command);
	      continue;
      }
      if(jcommand[0] == "HPUT") {
        command.type = CommandQueue::CommandType::HPUT;
        command_queue.push_command(command);
	      continue;
      }
      if(jcommand[0] == "HPOST") {
        command.type = CommandQueue::CommandType::HPOST;
        command_queue.push_command(command);
	      continue;
      }
      if(jcommand[0] == "DELETE") {
        command.type = CommandQueue::CommandType::DELETE;
        command_queue.push_command(command);
	      continue;
      }
      if(jcommand[0] == "EXEC") {
        command.type = CommandQueue::CommandType::EXEC;
        command_queue.push_command(command);
	      continue;
      }
      if(jcommand[0] == "GET") {
        command.type = CommandQueue::CommandType::GET;
        command_queue.push_command(command);
	      continue;
      }
      if(jcommand[0] == "SHOW") {
        command.type = CommandQueue::CommandType::SHOW;
        command_queue.push_command(command);
	      continue;
      }
      if(jcommand[0] == "DEFAULT") {
        command.type = CommandQueue::CommandType::DEFAULT;
        command_queue.push_command(command);
	      continue;
      }
      if(jcommand[0] == "SUB") {
        command.type = CommandQueue::CommandType::SUB;
        command_queue.push_command(command);
	      continue;
      }
      if(jcommand[0] == "UNSUB") {
        command.type = CommandQueue::CommandType::UNSUB;
        command_queue.push_command(command);
	      continue;
      }
      std::cout << "WARNING: Received unknown command " << jcommand[0] << ". Ignoring." << std::endl;
    }
    return session_continue;
  }

  std::string Session::send_P3(std::string p3_json) {
    (void)p3_json;
    return "{\"CMD\": [ [ \"END\" ] ] }";
  }

} // namespace
