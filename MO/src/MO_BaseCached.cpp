/** ***************************************************************************
 * Abstract class BaseCached
 * 
 * (c)2020 Christian Bendele
 *
 * See class description in header file
 *
 */

#include "MO_BaseCached.h"

#include <iostream>
#include <iomanip>

#include "Helper.h"

namespace Grandma {
namespace MO {

using namespace tinyxml2;
using namespace nlohmann;

BaseCached::BaseCached(std::string ddf_filename) {
  root.is_leaf = true;
  generate_tree_from_ddf(ddf_filename);
  std::cout << "CachedBase MO created tree from ddf: " << std::endl << std::setw(2) << serialize_json() << std::endl;
}

/**
 * @brief Set cached node from local application
 *
 * This method can be called by the local application to set the data of a node
 * in this MO's node tree.
 * It can also be called directly from the set_val callback of any derived class
 * as a minimal implementation for a completely static value store MO
 * This method will not check if any added node is valid according to the DDF file.
 * If this is required, the derived class needs to take care of it.
 *
 * @param[in] node_path - path (relative to this MO's root) of the node to set
 * @param[in] data - raw data (in a std::string) to write into the cached node
 * @param[in] add_missing_node - set to true to create non-existing nodes. If false, setting of non-existing nodes will silently fail (with a logged warning).
 */
void BaseCached::local_set_node(const std::string node_path, const std::string data, const bool add_missing_node) 
{
  // convert path into a vector of segments ("A/B/C" -> {"A", "B", "C"})
  const std::vector<std::string> path = Helper::vectorize_path(node_path); 

  Node *node = &root; // "iterator" used to point to the current node while descending into the tree

  for(auto segment : path) {
    auto node_it = find_if(node->children.begin(), node->children.end(), 
			    [&segment](const Node &child){return child.uri == segment;});
    if(node_it == node->children.end()) {
      if(add_missing_node) {
	Node temp_node;
	temp_node.uri = segment;
	node->children.push_back(temp_node);
	node_it = node->children.end()-1;
      } else {
	std::cout << "Warning: trying to set non-existing node " << node_path << std::endl;
	return;
      }
    }
    node = &*node_it;
  }
  node->data = data;
}

/**
 * @brief Get value of cached node for local application
 *
 * This method can be called by the local application to get the value (data) of 
 * a node in this MO's node tree.
 * It can also be called directly from the get_val callback of any derived class
 * as a minimal implementation for a completely static value store MO
 *
 * @param[in] node_path - path (relative to this MO's root) of the node to read
 * @return - data/value of the node if it exists, empty string otherwise
 *
 * Warning: This method can return an empty string both if this is the actual
 * data contained in the node, or if the node doesn't exist. There is no way for the
 * local application to differentiate. If this is not acceptable, the derived class
 * needs to take care of this. Rationale: The base classes try to provide a clean minimal
 * interface but also should not depend on support for exceptions. If derived classes
 * want to provide improved error recognition here, they can use exceptions and keep
 * the interface clean, or they can avoid exceptions but extend the interface. This
 * is not a decision I want to preempt in the base class.
 */
std::string BaseCached::local_get_node(const std::string node_path) 
const {

  // convert path into a vector of segments ("A/B/C" -> {"A", "B", "C"})
  const std::vector<std::string> path = Helper::vectorize_path(node_path); 

  const Node *node = &root; // "iterator" used to point to the current node while descending into the tree

  for(auto segment : path) {
    auto node_it = find_if(node->children.begin(), node->children.end(), 
			    [&segment](const Node &child){return child.uri == segment;});
    if(node_it == node->children.end()) {
      std::cout << "Warning: trying to get non-existing node " << node_path << std::endl;
      return "";
    }
    node = &*node_it;
  }
  return node->data;
}

/**
 * @brief Parse the ddf file and generate empty node structure in MO
 * 
 * This method will try to parse the given ddf file and will generate a structure
 * of Nodes (usually with empty data, but ddf files rarely will define default data)
 * starting at this class' root attribute. 
 * Most of the actual works is done in the recursive generate_node_from_ddf function
 * which is called from here.
 *
 * If the file can't be opened or can't be parsed (because of wrong content), it will
 * print a log message and exit without side effects.
 *
 * @param[in] filename - name of the ddf file
 */
void BaseCached::generate_tree_from_ddf(const std::string filename) {
  XMLDocument ddf_file;
  if(ddf_file.LoadFile(filename.c_str()) != XML_SUCCESS) {
    std::cout << "ERROR: BaseCached: Failed opening ddf file." << std::endl;
    return;
  }

  const XMLElement * xmlnode = ddf_file.FirstChildElement("MgmtTree");
  const std::string segment = xml_descend_safely(xmlnode, {"Node"});
  if(!xmlnode) {
    std::cout << "ERROR: BaseCached: Error parsing ddf file <MgmtTree><Node>..." << std::endl;
    return;
  }

  for(const XMLElement * child = xmlnode->FirstChildElement("Node"); child != NULL; child = child->NextSiblingElement("Node")) {
    root.is_leaf = false;
    root.children.push_back(generate_node_from_ddf(child));
  }
}

/**
 * @brief DDF File parsing: recursively descend into node structure
 *
 * This is the main method for parsing the ddf file into an empty MO node tree.
 * It descends recursively into the ddf file's xml structure, and generates
 * Node strucs into the MO's representation of the node tree (starting at this class' 
 * root attribute)
 *
 * @param[in] xmlnode - xml element on current level of recursion
 * @return Node struct repersenting input xmlnode and all its recursive child nodes
 */
BaseCached::Node BaseCached::generate_node_from_ddf(const XMLElement * const xmlnode) 
{
  Node node; // returned object of this method

  // according to DTD the <NodeName> child is mandatory (but may be empty). I have seen
  // at least one ddf file with this mandatory node missing in some Nodes, so we handle
  // this as if it was present but empty
  auto name_node = xmlnode->FirstChildElement("NodeName");
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
  auto format_node = xmlnode;
  std::string segment = xml_descend_safely(format_node, {"DFProperties","DFFormat"});
  if(format_node) {	// DFFormat is present -> use it to define if we are a leaf node
    if(format_node->FirstChildElement("node")) {
      node.is_leaf = false;
    } else {
      node.is_leaf = true;
    }
  } else { // DFFormat is not present -> fall back on checking if child nodes exist to define if we are a leaf node
    if(xmlnode->FirstChildElement("Node")) {
      node.is_leaf = false;
    } else {
      node.is_leaf = true;
    }
  }

  if(node.is_leaf) {
    if(xmlnode->FirstChildElement("Value") && xmlnode->FirstChildElement("Value")->GetText()) {
      node.data = xmlnode->FirstChildElement("Value")->GetText();
    } else {
      node.data = "";
    }
  } else { // recursively iterate over children of lower levels
    for(const XMLElement * child = xmlnode->FirstChildElement("Node"); child != NULL; child = child->NextSiblingElement("Node")) {
      node.children.push_back(generate_node_from_ddf(child));
    }
  }

  return node;
}

/**
 * @brief Helper function to descend xml element tree in a safe way.
 *
 * We often need to descend into the element tree of the xml file. However, using
 * chains like node->FirstChildElement("A")->FirstChildElement("B")->FirstChildElement("C")
 * is unsafe. If element <A> doesn't have a child <B>, the last call would cause an 
 * exception, since FirstElement("B") would return nullptr.
 * This is a helper function to savely replace these kind of chains without cluttering
 * the main function with error handling.
 *
 * @param[in,out] node - Input starting element. Output target element on success, NULL on failure
 * @param[in] path - each element in the vector is one level of descend. in the above example {"A", "B", "C"}
 *
 * @return in case of failure, name of the missing child element ("B" in the example above)
 */
std::string BaseCached::xml_descend_safely( const XMLElement*& node, 
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


/**
 * @brief Generate JSON object from this MO's node tree
 * 
 * This returns a JSON object suitable for serialization according to the format
 * described in OMA-DM Protocol Spec V2.0 section 7.2.1.4 - management object serialization.
 * The output of this method corresponds to the MOData object in this specification.
 * Most of the actual work is done in the recursive serialize_children() method called from here
 * 
 * This method is defined in the MO "Interface"
 */
json BaseCached::serialize_json() 
const {
  json json_mo;
  json_mo[root.uri] = serialize_children(root.children);
  return json_mo;
}

/**
 * @brief Recursively generate JSON object from this MO's node tree
 *
 * Recursive helper function for serialize_json()
 */
json BaseCached::serialize_children(std::vector<Node> children)
const {
  json json_object;
  for(Node &child : children) {
    if(child.is_leaf) {
      json_object[child.uri] = child.data;
    } else {
      json_object[child.uri] = serialize_children(child.children);
    }
  } 
  return json_object;
}


} // namespace
} // namespace
