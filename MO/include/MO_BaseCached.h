/** ***************************************************************************
 * Abstract class BaseCached
 * 
 * (c)2020 Christian Bendele
 *
 * This abstract class provides some of the necessary plumbing for managed
 * objects (MOs) that operate mostly on data that can be cached in the MO. 
 *
 * It is not meant as an appropriate base for MOs that operate on data that is
 * volatile from the device management application's point of view (e.g. sensor
 * measurements, log files) and/or heavily need to trigger immediate local action 
 * based on commands (get/set) from the backend.
 *
 * The provided plumbing currently can be grouped into four areas:
 *
 *  (1) A data structure to represent the data of the MO's node tree
 *  (2) Methods to generate the (empty) node tree from a ddf (xml) file
 *  (3) Methods to generate JSON objects from the node tree for use in serialization in the protocol
 *  (4) very simple local getter/setter methods
 * 
 * Not currently provided but planned for future versions are:
 *
 *  (5) Methods to verify operations (get/set) against the ddf file
 *
 * This base class has some compromises in handling of corner cases / external error
 * checking in the local getter/setter methods (4). For example, setting non-existing nodes 
 * with the local setter method either can create the missing node if requested, or it will fail
 * silently. There is no way of the caller (presumeably the local device management application) 
 * to know if the node existed or not. Similarly, trying to read a non-existing node will return an
 * empty string, giving he caller no way to differentiate this case from an existing node which 
 * actually contains an empty string as a value. My rationale here is that better external error 
 * checking would either need to (a) use exceptions or (b) clutter the interface with additional
 * output parameters or error-code style return values. I didn't want to preempt this decision, giving
 * architects of local applications the freedom to go either way. The local getter/setter methods in
 * this class are meant to be minimal solutions. If better external error checking is needed, 
 * derived classes need to take care of this. It is planned to provide some helper methods for
 * this in section (5) in the future.
 *
 */

#ifndef GRANDMA_MO_BASECACHED_H
#define GRANDMA_MO_BASECACHED_H

#include "MO_Interface.h"

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "tinyxml2.h"

namespace Grandma {
namespace MO {

class BaseCached : public Interface {

protected:

/**
 *  @{
 *  (1) A data structure to represent the data of the MO's node tree
 */
  struct Node {
    bool is_leaf;
    std::string uri;
    std::string data;
    std::vector<Node> children;
  };

  Node root;

/** 
 *  @}
 *  @{
 *  (2) Methods to generate the (empty) node tree from a ddf (xml) file
 */
public:
  BaseCached(std::string ddf_filename);
protected:
  Node generate_node_from_ddf(const tinyxml2::XMLElement * const xml_node);
  void generate_tree_from_ddf(std::string filename);
  std::string xml_descend_safely(const tinyxml2::XMLElement*& node, const std::vector<std::string> &path) const;

/** 
 *  @}
 *  @{
 *  (3) Methods to generate JSON objects from the node tree for use in serialization in the protocol
 */
public:
  // this method is defined in the MO Interface
  virtual nlohmann::json serialize_json() const;
protected:
  nlohmann::json serialize_children(std::vector<Node> children) const;

/** 
 *  @}
 *  @{
 *  (4) very simple local getter/setter methods
 */
public:
  void local_set_node(const std::string node_path, const std::string data, const bool add_missing_node = false);
  std::string local_get_node(const std::string node_path) const;

/** 
 *  @}
 *  @{
 *  (5) Methods to verify operations (get/set/exec) against the ddf file
 */
// TODO

};

/**
 * @}
 */

} // namespace
} // namespace

#endif
