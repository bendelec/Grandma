/**
 * MO (managed object) tree for Grandma OMA-DM client library
 *
 * (c)2020 Christian Bendele
 *
 * This class contains the internal representation of the MO tree for the client library.
 *
 * It provides methods to register MO types (with a corresponding ddf file) to the client
 * library and to add instances of MOs for registered MO types.
 *
 * It further provides methods for serializaion of the MO tree structure and its contents,
 * that are 
 *
 * TODO: Should introduce a result / error enum instead of returning bool in most methods
 * to facilitate better problem handling. Planned for future iteration.
 *
 */

#ifndef GRANDMA_MOTREE_H
#define GRANDMA_MOTREE_H

#include <map>
#include <vector>
#include <memory>

#include <nlohmann/json.hpp>

#include "MO_Interface.h"


namespace Grandma {
class MOTree {

  // for each MO Type, store the canonical download URL of the ddf file so it can be 
  // reported to the backend in the DDF field of the MOS vector entry
  // and a list of instances of this MO type
  struct MOType {
    std::string ddf_url;
    std::vector<std::shared_ptr<MO::Interface> > miids;
  };

  // the key of the MOs map is the urn as it is defined <DDFName> of the ddf files 
  // root Node. Example is "urn:oma:mo:oma-dm-devinfo:1.2"
  std::map<std::string, MOType> MOs;

public:

  bool register_DDF(std::string urn, std::string filename, std::string ddf_url = "");

  bool add_MO(std::string urn, std::shared_ptr<MO::Interface> mo);



  nlohmann::json p1_MOS_json() const;

  nlohmann::json dump_serialized_MOS() const;

};

} // namespace

#endif
