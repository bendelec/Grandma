#include "DMClient.h"

#include <iostream>
#include <iomanip>

namespace Grandma {

DMClient::DMClient() : session(motree) {}

/**
 * Pass through to MOTree - see there for documentation
 */
void DMClient::register_DDF(std::string urn, std::string filename, std::string ddf_url) {
  motree.register_DDF(urn, filename, ddf_url);
}

/**
 * Pass through to MOTree - see  there for documentation
 */
void DMClient::add_MO(std::string urn, std::shared_ptr<MO::Interface> mo, std::string miid) {
  motree.add_MO(urn, mo, miid);
}

// TODO: this is very rough/early/proof of concept
// some of this should possibly be moved into Session class, but that is even less
// finished at this moment, so it can stay here until the design gets more refined.
void DMClient::start_session(bool server_initiated) {

  if(server_initiated) {
    alert_queue.add_alert(Alert("urn:oma:at:dm:2.0:ServerInitiatedMgmt"));
  } else {
    alert_queue.add_alert(Alert("urn:oma:at:dm:2.0:ClientInitiatedMgmt"));
  }

  nlohmann::json P1_json;
  P1_json["MOS"] = motree.p1_MOS_json();
  P1_json["Alert"] = alert_queue.p1_Alert_json();

  if(P1_dump_tree) {
    P1_json["MgmtTree"] = motree.dump_serialized_MOS();
  }

  std::cout << "Sending P1 to Server:" << std::endl << std::setw(2) << P1_json << std::endl;

  std::string P2_json = session.send_P1(P1_json.dump());

  while(session.parse_P2(P2_json)) {
    P2_json = session.send_P3("");
  }
}

void DMClient::set_P1_dump_tree(bool enable) {
  P1_dump_tree = enable;
}

void DMClient::set_device_id(std::string id) {
  DevId = id;
}

} // namespace

