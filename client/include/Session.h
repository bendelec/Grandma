#ifndef GRANDMA_SESSION_H
#define GRANDMA_SESSION_H

#define CPPHTTPLIB_OPENSSL_SUPPORT

#include <nlohmann/json.hpp>
#include <httplib/httplib.h>

#include "CommandQueue.h"
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

  MOTree &motree; // TODO: moving out command handlers from session class should make this unnecessary and improve soc
  CommandQueue &command_queue;

public:

  Session(MOTree &motree, CommandQueue &command_queue);

  std::string send_P1(std::string p1_json);
  bool parse_P2(std::string p2_json);
  std::string send_P3(std::string p3_json);

};

} // namespace

#endif
