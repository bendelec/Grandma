/**
 * MO (managed object) handler for Grandma OMA-DM client library
 *
 * (c)2020 Christian Bendele
 *
 * This class handles the parsing of the ddf file to support a MO type, and provides
 * methods for accessing, iterating, managing instances of MO::Interface, which are
 * the actual MO implementation that are part of the local application
 *
 * TODO: Should introduce a result / error enum instead of returning bool in most methods
 * to facilitate better problem handling. Planned for future iteration.
 *
 */

#ifndef GRANDMA_MO_HANDLER_H
#define GRANDMA_MO_HANDLER_H

#include <nlohmann/json.hpp>
#include "tinyxml2.h"

#include "MO_Interface.h"

namespace Grandma {

class MOHandler {

  struct Node {
    bool is_leaf;
    std::string uri;
    std::vector<Node> children;
    //TODO: add type, access permissions, etc, not yet implemented.
  };

  std::string urn;
  Node root;	    // root of DDF file derived node tree
  std::string ddf_url;  // canonical download URL of ddf file
  unsigned next_miid;

  // The key of the intances (MI) map is the  miid
  std::map<std::string, std::shared_ptr<MO::Interface> > instance;
  
public:

  MOHandler(std::string urn, std::string url);
  std::string url() const;

  bool generate_tree_from_ddf(std::string filename);
  nlohmann::json build_MOData(std::string uri, std::shared_ptr<MO::Interface>) const;
  nlohmann::json serialize_MIs() const;
  nlohmann::json p1_MOS_json() const;
  
  // TODO: change type to enum
  int check_access(std::string uri) const; 

  bool node_set(const std::string uri, const nlohmann::json modata);
  bool node_get(const std::string uri, nlohmann::json &modata);

  bool add_instance(std::shared_ptr<MO::Interface> mo, std::string miid);

private:

  // TODO: code replication in this methods from MO_BaseCached. Try to refactor without breaking the
  // logical source barrier between library (this) and local application (MO base classes)?
  Node generate_node_from_ddf(const tinyxml2::XMLElement * const xml_node);
  std::string xml_descend_safely(const tinyxml2::XMLElement*& node, const std::vector<std::string> &path) const;
  nlohmann::json serialize_children(std::shared_ptr<MO::Interface> mo, std::vector<Node> children, std::string uri_prefix) const;
};

} //namespace

#endif
