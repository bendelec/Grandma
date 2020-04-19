#include "MOHandler.h"
#include "Helper.h"

#include <iostream>

namespace Grandma {

using namespace tinyxml2;
using namespace nlohmann;

MOHandler::MOHandler(std::string urn, std::string url) : urn(urn), ddf_url(url), next_miid(1) {
  // TODO: This is mostly a minimal dummy for now. We should at least open and parse the 
  // DDF file and check if the <DDFName> in the file, if present, corresponds to the given urn.
  // unfortunately some ddf files, including from the official oma homepage, seem to be missing
  // the <DDFName> node. In this case, we should probably trust the user (ouch) and accept the
  // registration, relying on downstream error tolerance (on node access) to minimize impact 
  // in case it doesn't match after all.
 
}

// TODO: Not yet doing anything, just an empty method shell to make the linker happy
// and allow building
bool MOHandler::node_set(const std::string uri, const json modata) {
    std::cout << "MOHandler::node_set, uri = " << uri << ", modata:" << std::endl;
    std::cout << modata.dump(2) << std::endl;
    auto delim = uri.find("/");
    std::string miid = uri.substr(0, delim);
  
    auto mi = instance.find(miid);
    if(mi == instance.end()) {
      std::cout << "Warning: MOHandler::node_set() - MMO Type " << urn << " has no miid " << miid << std::endl;
      return false;
    }

    Node *node = find_node(Helper::vectorize_path(uri.substr(delim)));
    if(!node) {
      std::cout << "Warning: MOHandler::node_set - Node " << uri.substr(delim) << " does not exist in MO type " << urn << std::endl;
    }

    // TODO - check type, access right, etc...
    
    (void)modata;
    
      
    return false;
    
}

MOHandler::Node *MOHandler::find_node(std::vector<std::string> path, Node *node) {
  if(!node) node = &root;
  for(auto segment : path) {
    auto it = find_if(node->children.begin(), node->children.end(),
          [&segment](const Node &child){return child.uri == segment;});
    if(it == node->children.end()) {
      std::cout << "Warning: MOHandler_find_node - node " << node->uri << " has no child " << segment << std::endl;
      return nullptr;
    }
    node = &*it;
  }
  return node;
}

/**
 * Provide MOS json object for for package P1 (structure)
 *
 * The main method is in MOTree, this sub-method provides the MIID sub-vector for one MO
 */
json MOHandler::p1_MOS_json() 
const {
  json mo;
  if(ddf_url != "") {
    mo["DDF"] = ddf_url;
  }
  mo["MOID"] = urn;
  for(auto &mi : instance) {
    mo["MIID"].push_back(mi.first);
  }
  return mo;
}

/**
 * Provide serialization of MO instances in this MO type (contents)
 *
 * provide a json object containing a vector of serialization objects of each 
 * of the MO instances of this type, in the same order as provided by above method for
 * package P1 MOS vector. Each serialization object corresponds to the schema specified in
 * section 7.2.1.4 of OMA-DM 2.0 protocol specification
 */
json MOHandler::serialize_MIs() 
const {
  json mos;

  for(auto mi : instance) {
    json mo; json modata;
    
    modata[root.uri] = serialize_children(mi.second, root.children, "");
    mo["MOData"] = modata;
    mos.push_back(mo);
  }
  return mos;
}

/**
 * Provide (recursively) serialization of an MO node (and its child nodes)
 *
 * @param[in] mo - MO::interface object representing an actual instance of an MO implemented by the local application
 * @param[in] children
 * @param[in] uri-prefix - used to carry the full path to the current node through the recursion
 *
 * FIXME: clean up, decide const-ness of parameters, complete documentation
 */
json MOHandler::serialize_children(std::shared_ptr<MO::Interface> mo, std::vector<Node> children, std::string uri_prefix)
const {
  json json_object;

  for(Node &child : children) {
    if(child.is_leaf) {
      bool exists = true; bool valid = true;
      std::string value = mo->get_val(uri_prefix + "/" + child.uri, exists, valid);
      if(exists && valid) {
	json_object[child.uri] = value;
      }
    } else {
      json node = serialize_children(mo, child.children, uri_prefix + "/" + child.uri);
      if(node != nullptr) {
	json_object[child.uri] = node;
      }
    }
  }
  return json_object;
}

/** 
 * @brief add an instance of an MO of this type to the tree
 *
 */
bool MOHandler::add_instance(std::shared_ptr<MO::Interface> mo, std::string miid) {
  // first decide on a miid
  if(miid == "") { // find our own
    do miid = std::to_string(next_miid++); while(instance.find(miid) != instance.end());
  } else { // check if user provided miid is unique
    if(instance.find(miid) != instance.end()) return false;
  }
  if(mo->check_ddf_name_compatibility(urn)) {
    instance[miid] = mo;
    return true;
  } else {
    std::cout << "ERROR: MO object declares not compatible with urn " << urn << std::endl;
    return false;
  }
}

/**
 * @brief Parse the ddf file and generate node structure in MO
 * 
 * This method will try to parse the given ddf file and will generate a structure
 * of Nodes starting at this class' root attribute. 
 * Most of the actual works is done in the recursive generate_node_from_ddf function
 * which is called from here.
 *
 * If the file can't be opened or can't be parsed (because of wrong content), it will
 * print a log message and exit without side effects.
 *
 * @param[in] filename - name of the ddf file
 */
bool MOHandler::generate_tree_from_ddf(const std::string filename) {
  XMLDocument ddf_file;

  if(ddf_file.LoadFile(filename.c_str()) != XML_SUCCESS) {
    std::cout << "ERROR: BaseCached: Failed opening ddf file." << std::endl;
    return false;
  }

  const XMLElement * xmlnode = ddf_file.FirstChildElement("MgmtTree");
  const std::string segment = xml_descend_safely(xmlnode, {"Node"});
  if(!xmlnode) {
    std::cout << "ERROR: BaseCached: Error parsing ddf file <MgmtTree><Node>..." << std::endl;
    return false;
  }

  // according to DTD the <NodeName> child is mandatory (but may be empty). I have seen
  // at least one ddf file with this mandatory node missing in some Nodes, so we handle
  // this as if it was present but empty
  auto name_node = xmlnode->FirstChildElement("NodeName");
  if(name_node && name_node->GetText()) {
    root.uri = name_node->GetText();
  } else {
    // TODO: many assumptions... should this be made safer?
    root.uri = urn.substr(15);
    root.uri = root.uri.substr(0, root.uri.find(":"));
  }


  for(const XMLElement * child = xmlnode->FirstChildElement("Node"); child != NULL; child = child->NextSiblingElement("Node")) {
    root.is_leaf = false;
    root.children.push_back(generate_node_from_ddf(child));
  }
  return true;
}

/**
 * @brief DDF File parsing: recursively descend into node structure
 *
 * This is the main method for parsing the ddf file into the MO node structure.
 * It descends recursively into the ddf file's xml structure, and generates
 * Node strucs into the MO's representation of the node tree (starting at this class' 
 * root attribute)
 *
 * @param[in] xmlnode - xml element on current level of recursion
 * @return Node struct repersenting input xmlnode and all its recursive child nodes
 */
MOHandler::Node MOHandler::generate_node_from_ddf(const XMLElement * const xml_node) {
   Node node; // returned object of this method

  // according to DTD the <NodeName> child is mandatory (but may be empty). I have seen
  // at least one ddf file with this mandatory node missing in some Nodes, so we handle
  // this as if it was present but empty
  auto name_node = xml_node->FirstChildElement("NodeName");
  if(name_node && name_node->GetText()) {
    node.uri = name_node->GetText();
  } else {
    node.uri = "*";
  }

  // each <Node> Element can represent either a leaf node or an interior node.
  // 
  // If the <DFProperties><DFFormat> Element exists, it will containt a <node> child element
  // for interior nodes, and any other Element (e.g. <chr>) chile element for leaf nodes
  // 
  // according to DTD the <DFProperties> tree is not "required". If it is missing, we fall back
  // on checking if the current <Node> has any child <Node>s or not in order to define it as 
  // a interior node or leaf not respectively.
  auto format_node = xml_node;
  std::string segment = xml_descend_safely(format_node, {"DFProperties","DFFormat"});
  if(format_node) {	// DFFormat is present -> use it to define if we are a leaf node
    if(format_node->FirstChildElement("node")) {
      node.is_leaf = false;
    } else {
      node.is_leaf = true;
    }
  } else { // DFFormat is not present -> fall back on checking if child nodes exist to define if we are a leaf node
    if(xml_node->FirstChildElement("Node")) {
      node.is_leaf = false;
    } else {
      node.is_leaf = true;
    }
  }

  if(node.is_leaf) {
    // TODO: parse access permisions etc
  } else { // recursively iterate over children of lower levels
    for(const XMLElement * child = xml_node->FirstChildElement("Node"); child != NULL; child = child->NextSiblingElement("Node")) {
      node.children.push_back(generate_node_from_ddf(child));
    }
  }

  return node;
}


// this is a consideration for inclusion in the Helper class, however, that would 
// inject tinyxml2 dependency into local application if it uses the Helper class...
// decisions, decisions...
std::string MOHandler::xml_descend_safely( const XMLElement*& node, 
					    const std::vector<std::string> &path) 
const {
  if(node) { // sanity check input
    for(auto &&segment : path) {
      node = node->FirstChildElement(segment.c_str());
      if(!node) {
	return segment;
      }
    }
  } else {
    return "<start of descend>"; // can't return anything more useful if initial node was null
  }
  return "";
}


} // namespace
