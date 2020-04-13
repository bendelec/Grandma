/**
 * AlertQueue
 *
 * (c) 2020 Christian Bendele
 */

#include "AlertQueue.h"

using namespace nlohmann;

namespace Grandma {
 
Alert::Alert(std::string AlertType) : AlertType(AlertType), in_transit(false) {}

json AlertQueue::p1_Alert_json() {
  json alerts;
  for(auto &alert : alert_queue) {
    if(!alert.in_transit) {
      json al;
      al["AlertType"] = alert.AlertType;
      alerts.push_back(al);
      alert.in_transit = true;
    }
  }
  return alerts;
};

void AlertQueue::add_alert(Alert alert) {
  alert_queue.push_back(alert);
}

} // namespace
