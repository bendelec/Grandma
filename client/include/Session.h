#ifndef GRANDMA_SESSION_H
#define GRANDMA_SESSION_H

#define CPPHTTPLIB_OPENSSL_SUPPORT

#include <nlohmann/json.hpp>
#include <httplib/httplib.h>

#include "MOTree.h"

namespace Grandma {

class Session {
  enum class SessionState {
    Unknown,
    Idle,
    P1,
    P2,
    P3cont,
    P3end
  };

   // TODO move this out to command queue
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

  MOTree &motree; // TODO: moving out command handlers from session class should make this unnecessary and improve soc

public:

  Session(MOTree &motree);

  std::string send_P1(std::string p1_json);
  bool parse_P2(std::string p2_json);
  std::string send_P3(std::string p3_json);

private:
  // TODO: these command handlers should be factored out either into their own class(es). 
  // Or into the command queue class (need to decide this) They don't belong 
  // into Session class
  // in this case check if #include <nlohmann/json.hpp> can be removed from this file
  void do_hget(nlohmann::json command);

};

} // namespace

#endif
