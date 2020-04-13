#include <memory>

#include "DMClient.h"
// #include "MO_DevInfo.h"
#include "MO_StaticData.h"
// #include <nlohmann/json.hpp>

using namespace Grandma;

int main(int argc, char *argv[]) {
  (void)argc; (void)argv;

  DMClient client;
  client.set_device_id("PlanB");
  client.set_P1_dump_tree(true);
  
  client.register_DDF("urn:oma:mo:oma-dm-devinfo:1.2", "../ddf/DevInfo.ddf", "http://localhost/dummy.ddf");
  client.register_DDF("urn:oma:mo:oma-dm-dmacc:1.2", "../ddf/DMAcc.ddf");
  client.register_DDF("urn:oma:mo:oma-sessioninfomo:1.0", "../ddf/SessionInfo.ddf");
  client.register_DDF("urn:oma:mo:oma-fumo:1.0", "../ddf/Fumo.ddf");

  std::shared_ptr<MO::StaticData> mo_devinfo = std::make_shared<MO::StaticData>("devinfo", "../ddf/DevInfo.ddf");
  mo_devinfo->local_set_node("DevID", "PlanB");
  mo_devinfo->local_set_node("Man", "Continental China");
  mo_devinfo->local_set_node("Mod", "Smartroad ECU");
  mo_devinfo->local_set_node("OEM", "Continental");
  // mo_devinfo->local_set_node("Blah/P0_URL", "http://localhost", true); // TODO: need to fix this case in local_set_node...
  mo_devinfo->local_set_node("Ext/P0URL", "http://localhost", true);

  std::shared_ptr<MO::StaticData> mo_sessinfo = std::make_shared<MO::StaticData>("sessioninfomo", "../ddf/SessionInfo.ddf");
  mo_sessinfo->local_set_node("CBT", "3");
  mo_sessinfo->local_set_node("ROAMING", "2");

  std::shared_ptr<MO::StaticData> mo_dmacc = std::make_shared<MO::StaticData>("dm-dmacc", "../ddf/DMAcc.ddf");
  
  std::shared_ptr<MO::StaticData> mo_fumo = std::make_shared<MO::StaticData>("fumo", "../ddf/Fumo.ddf");

  client.add_MO("urn:oma:mo:oma-dm-devinfo:1.2", mo_devinfo);
  client.add_MO("urn:oma:mo:oma-dm-dmacc:1.2", mo_dmacc);
  client.add_MO("urn:oma:mo:oma-sessioninfomo:1.0", mo_sessinfo);
  client.add_MO("urn:oma:mo:oma-fumo:1.0", mo_fumo, "rootfs");
  client.add_MO("urn:oma:mo:oma-fumo:1.0", mo_fumo, "apps");

//  client.finish_bootstrap();

  client.start_session();

  return 0;
}
