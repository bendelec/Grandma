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
#include "MOHandler.h"

namespace Grandma {
class MOTree {

  // the key of the MOs map is the urn as it is defined <DDFName> of the ddf files 
  // root Node. Example is "urn:oma:mo:oma-dm-devinfo:1.2"
  std::map<std::string, MOHandler> MOs;

public:

  bool register_DDF(const std::string urn, const std::string filename, const std::string ddf_url = "");
  bool add_MO(const std::string urn, std::shared_ptr<MO::Interface> mo, const std::string miid);

  bool node_set(const std::string uri, const nlohmann::json modata);
  bool node_get(const std::string uri, nlohmann::json &modata); // not const on purpose to allow side effects

  nlohmann::json p1_MOS_json() const;
  nlohmann::json dump_serialized_MOS() const;

};

} // namespace

#endif
