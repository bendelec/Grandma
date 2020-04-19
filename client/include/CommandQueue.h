#ifndef GRANDMA_COMMANDQUEUE_H
#define GRANDMA_COMMANDQUEUE_H

#include <deque>
#include <nlohmann/json.hpp>

#include "MOTree.h"

namespace Grandma {

class CommandQueue {

public:
  enum class CommandType {
    END,
    CONT,
    HGET,
    HPUT,
    HPOST,
    DELETE,
    EXEC,
    GET,
    SHOW,
    DEFAULT,
    SUB,
    UNSUB
  };

  struct Command {
    CommandType type;
    std::vector<std::string> parameter;
  };

private:

  struct Status {
    unsigned code;
    std::vector<std::string> URI;

    Status(unsigned code);
  };

  std::deque<Command> commands;
  std::deque<Status> responses;

  struct URL {
    enum class Protocol {
      HTTP,
      HTTPS
    } protocol;
    std::string server;
    int port;
    std::string path;
    std::string query;

    bool parse_from_string(std::string s_uri);
  };

  MOTree &motree; 

public:
  CommandQueue(MOTree &motree);

  void push_command(Command);

  void do_commands();

  nlohmann::json p3_SC_json();

private:

  void do_hget(std::vector<std::string> params);

};

} // namespace

#endif
