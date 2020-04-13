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
   * Set a node in a MO instance
   *
   * The actual work is done in the MOHandler, this will just select the correct
   * handler instance for the given urn (see uri parameter) and pass on miid/path
   * to the node_set method of the MOHandler
   *
   * @param[in] uri - "<urn>/<miid>/<path>"
   * @param[in] modata - json object with mo data serialization according to protocol standard
   * @return true on success
   *
   * TODO: need to support condition reporting by additional output parameters, similar
   * to what he get/set methods in MO::Interface support, so we can have proper response
   * codes for different error cases
   **/
  bool MOTree::node_set(const std::string uri, const json modata) {
    auto delim = uri.find("/");
    std::string urn = uri.substr(0, delim);
    std::string path = uri.substr(delim);

    auto mo = MOs.find(urn);
    if(mo == MOs.end()) {
      return false;
    }
    return mo->second.node_set(path, modata);
  }

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
  bool MOTree::register_DDF(const std::string urn, const std::string filename, const std::string ddf_url) 
  {
    if(MOs.find(urn) == MOs.end()) {
      std::cout << "Registering DDF File " << filename << " for " << urn << std::endl;

      // TODO reconsider/discuss use of exceptions and proper RAII...
      MOHandler handler(urn, ddf_url);
      if(handler.generate_tree_from_ddf(filename)) {
	      MOs.insert(std::make_pair(urn, handler));
      } else {
      	std::cout << "ERROR: DDF file couldn't be parsed - not registering " << urn << std::endl;
	      return false;
      }
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
   * @param[in] optional miid - user defined miid. if unset, library will assign a unique miid.
   */
  bool MOTree::add_MO(std::string urn, std::shared_ptr<MO::Interface> mo, std::string miid) 
  {
    auto handler = MOs.find(urn);
    if(handler == MOs.end()) {
      std::cout << "Error: No DDF registered for " << urn << std::endl;
      return false;
    } else {
      return handler->second.add_instance(mo, miid);
    }
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
      json mo = MO.second.p1_MOS_json();
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
    json mos = json::array();
    // iterate over MO types
    for(auto &MO : MOs) {
      json mo = MO.second.serialize_MIs();
      mos.insert(mos.end(), mo.begin(), mo.end());
    }
    return mos;
  }

} // namespace
