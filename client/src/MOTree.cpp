/**
 * MO (managed object) tree for Grandma OMA-DM client library
 *
 * (c)2020 Christian Bendele
 *
 * See class description in header file
 *
 */
#include "MOTree.h"
#include <iostream>
#include <utility>

namespace Grandma {

using namespace nlohmann;

  /**
   * Register a new MO Type and its corresponding DDF file
   *
   * This needs to be called before adding an MO instance of the respective type.
   *
   * @param[in] urn - corresponding to DDFName in ddf file (ex. "urn:oma:mo:oma-dm-devinfo:1.2")
   * @param[in] filename - filename of a local copy of the DDF file defining MOs of this type
   * @param[in] ddf_url - canonical download URL for this ddf file. If provided, this URL should be reachable for the servers that this device connects to, but doesn't need to be reachable to the device itself
   *
   * @return true if successful, false on error (e.g. can't open file etc...)
   *
   * This method will report success if called with an already registered urn with
   * the same file name (consistent double-registration) but it will return error
   * if called with an already registered urn with a different filename (inconsistent
   * double-registration (TODO: not currently implemented)
   */
  bool MOTree::register_DDF(std::string urn, std::string filename, std::string ddf_url) 
  {
    (void) filename;

    // TODO: This is mostly a minimal dummy for now. We should at least open and parse the 
    // DDF file and check if the <DDFName> in the file, if present, corresponds to the given urn.
    // unfortunately some ddf files, including from the official oma homepage, seem to be missing
    // the <DDFName> node. In this case, we should probably trust the user (ouch) and accept the
    // registration, relying on downstream error tolerance (on node access) to minimize impact 
    // in case it doesn't match after all.
    std::string ddf_name = urn;

    if(MOs.find(ddf_name) == MOs.end()) {
      std::cout << "Registering DDF File " << filename << " for " << ddf_name << std::endl;
      MOs.insert(std::make_pair(ddf_name, MOType() ));
      MOs[ddf_name].ddf_url = ddf_url;
    } 
    // TODO: should only tolerate double-registration if the same filename is given. For now,
    // silently ignore all double-registration
    
    return true;
  }

  /**
   * Add a new MO instance to the MO Tree
   *
   * Before adding an instance using this method, the MO type needs to be registered
   * using register_DDF. The client library will verify if the MO instance given is compatible
   * with the given urn by calling mo->check_ddf_name_compatibility() 
   *
   * @param[in] urn - corresponding to DDFName in ddf file (ex. "urn:oma:mo:oma-dm-devinfo:1.2")
   * @param[in] pointer to an object implementing MO::Interface that supports an MO according to urn
   */
  bool MOTree::add_MO(std::string urn, std::shared_ptr<MO::Interface> mo) 
  {
    if(MOs.find(urn) == MOs.end()) {
      std::cout << "Error: No DDF registered for " << urn << std::endl;
      return false;
    }

    if(mo->check_ddf_name_compatibility(urn)) {
      MOs[urn].miids.push_back(mo);
    } else {
      std::cout << "Error: provided MO object doesn't support " << urn << std::endl;
      return false;
    }
    // TODO: in this place, call mo.init_mo() or only once all MOs are added on first session start?

    return true;
  }

  /**
   * Provide MOS json object for for package P1 (structure)
   *
   * provide a json object according to the MOS field of package P1 as specified
   * in section 7.2.1.1 of OMA-DM 2.0 protocol specification
   */
  json MOTree::p1_MOS_json() 
  const {
    json mos;
    // iterate over MO types
    for(auto &MO : MOs) {
      json mo;
      if(MO.second.ddf_url != "") {
	mo["DDF"] = MO.second.ddf_url;
      }
      mo["MOID"] = MO.first;
      // iterate over MO instances (miids)
      for(auto &MI : MO.second.miids) {
	mo["MIID"].push_back(MI->miid());
      }
      mos.push_back(mo);
    }

    return mos;
  }

  /**
   * Provide full serialization of MO tree (contents)
   *
   * provide a json object containing a vector of serialization objects of each 
   * of the MO instances in the tree, in the same order as listed in package P1
   * MOS vector. Each serialization object corresponds to the schema specified in
   * section 7.2.1.4 of OMA-DM 2.0 protocol specification
   */
  json MOTree::dump_serialized_MOS() 
  const {
    json mos;
    // iterate over MO types
    for(auto &MO : MOs) {
      for(auto &MI : MO.second.miids) {
	json mo;
	mo["MOData"] = MI->serialize_json();
	mo["ClientURI"] = MO.first + "/" + MI->miid() + "/";
	mos.push_back(mo);
      }
    }
    return mos;
  }

} // namespace
