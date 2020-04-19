/**
 * Alert queue for Grandma OMA-DM client
 *
 * (c) Christian Bendele
 *
 * TODO: unfinished architecture. Rewrite comment once architecture more clear
 *
 * This class will manage the queue of outstanding Alerts.
 *
 * It will contain methods for adding alerts and for serializing them for sending
 * them to the server in P1 and P3. 
 *
 * It should later contain some logic for keeping track which alerts were 
 * successfully sent to the server, and then remove them from the queue (TODO)
 *
 */

#ifndef GRANDMA_ALERTQUEUE_H
#define GRANDMA_ALERTQUEUE_H


#include <deque>

#include <nlohmann/json.hpp>


namespace Grandma {

struct Alert {

  std::string AlertType;
  std::string SourceURI;
  std::string TargetURI;
  std::string Mark;
  std::string DataType;
  std::string Data;

  bool in_transit;

  Alert(std::string AlertType); 
};

class AlertQueue {

  std::deque<Alert> alert_queue;

public:

  nlohmann::json package_alert_json();

  void add_alert(Alert alert);

};

} // namespace

#endif
