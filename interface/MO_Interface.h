/**
 * Interface definition for managed object classes (MOs)
 *
 * (c)2020 Christian Bendele
 *
 * This specifies the interface that all MO classes need to provide. Objects implementing
 * this interface can be added to the MO tree of the Grandma OMA-DM protocol client library
 * using the DMClient::addMO() method.
 *
 * The methods defined by this interface can be grouped into the following areas:
 *
 *  (1) callbacks called by the protocol client library as a direct result of protocol commands
 *  (2) callbacks called by the protocol client library as part of Session or MO management
 *
 */
#ifndef GRANDMA_MO_INTERFACE_H
#define GRANDMA_MO_INTERFACE_H

#include <string>
#include <nlohmann/json.hpp>

namespace Grandma {
namespace MO {

class Interface {
public:

  /**
   *  @{
   *  (1) callbacks called by the protocol client library as a direct result of protocol commands
   */
  
  /**
   * @brief callback for reading the value of a node
   *  
   * This will be called by the protocol client library to read the value of a node in the MO
   * instance. The call of this method will usually be the result of a GET, HPUT or HPOST
   * command received by the client library in a protocol session with the DM backend,
   * however the client library may call this method at any moment at its own discretion.
   *
   * @param[in] node_path - path (relative to this MO's root) of the node to update
   * @param[out] node_exists -	May be set to false if the requested node doesn't exist. 
   *				The client library will set this to true before invoking the callback. 
   *				Protocol hint: The protocol client will respond to the backend command 
   *				with "404 - Not found" if this is set to false by the callback
   * @param[out] valid_data - May be set to false if the requested node exists, but no valid data can
   *				currently be provided. 
   *				The client library will set this to true before invoking the callback. 
   *				This can be used for nodes representing volatile signals like live sensor 
   *				data. Protocol hint: If this is set to false by the callback the protocol
   *				client will respond to backend GET commands with "204 - No content".
   *				TODO: define behaviour for HPUT and HPOST calls if not valid_data
   *				
   * @return Shall return the current value of the node specified, or undefined if node_exists of valid_data are set to false
   *
   * This is deliberately not declared const to allow implementation of side effects in the
   * MO class
   */
  virtual std::string get_val(const std::string node_path, bool &node_exists, bool &valid_data) = 0;
  
  /**
   * @brief callback for writing the value of a node
   *
   * This will be called by the protocol client library to update the value of a node in
   * the MO instance. The call of this method will always be the direct result of an HGET
   * comment received by the client library in a protocol session with the DM backend.
   *
   * @param[in] node_path - path (relative to this MO's root) of the node to update
   * @return Shall return true if the node was successfully updated, false if updating was not possible
   */
  virtual bool set_val(const std::string node_path, const std::string data) = 0;

  /**
   * @brief callback for deleting a node from the MOs node tree
   *
   * This will be called by the protocol client library to remove a node from the node tree
   * of the MO instance. The call of this method will always be the direct result of a DELETE
   * comment received by the client library in a protocol session with the DM backend.
   *
   * @param[in] node_path - path (relative to this MO's root) of the node to remove
   * @return Shall return true if the node was successfully removed OR never existed. False if deletion of the node was not possible.
   */
  virtual bool remove_node(const std::string node_path) = 0;

  /**
   * @brief callback for reading the value of a node
   * 
   * This will be called by the protocol client as the direct result of an EXEC command
   * received by the client library in a protocol session with the DM backend.
   *
   * @param[in] node_path - path (relative to this MO's root) of the node to execute
   * @return Shall returns true if the execute command was accepted. This does not imply successful execution, only acceptance of the command. Shall return false if the execute command can not be accepted
   *  
   * TODO: the following section is preliminary and incomplete
   * In order to report the result of the execute command to the backend, any MO class
   * implementing this interface must ensure that it will call the DMClient::XXX() method 
   * exactly once for each time this callback is invoked by the client library and the
   * callback returned true.
   *
   * For each invocation of the execute callback by the client library the implementation
   * can call DMClient::XXX() either from inside its implementation of the callback
   * for synchronous reporting, or at *any* time after returning from the callback 
   * (see section 5.4.1 of the OMA DM 2.0 protocol specification) 
   * 
   * TODO: check if additional info (parameters) are needed in order
   * to correctly link the response (synchronous or asynchronous) with the command.
   * 
   * This interface is not considered stable yet
   */
  virtual bool execute(const std::string node_path) = 0;


  /**
   *  @}
   *  @{
   *  (2) callbacks called by the protocol client library as part of Session or MO management
   */

  /**
   * @brief callback for the protocol client library to check MO compatibility
   *
   * The protocol client library will call this before adding an instance of an MO implementation
   * to its MO tree in order to check if the given implementation of the MO class is compatible with
   * the DDF file registered with the client library. 
   *
   * @param[in] ddfname - DDFName as given in the respective name in a DDF file. Example: "urn:oma:mo:oma-dm-dmacc:1.2"
   * @return - MO implementation classes should only return true if they are compatible with the DDF file corresponding to the given DDFName
   * 
   */
  virtual bool check_ddf_name_compatibility(std::string ddfname) = 0;

  /**
   * @brief callback on adding the MO instance to the client library's MO tree
   *
   * The protocol client library will call this after successfully adding the instance of the MO
   * into its MO tree.
   */
  virtual void init_mo() = 0; // gets called by the client after adding an instance to the tree.

  /**
   * @brief callback on removing the MO instance from the client library's MO tree
   *
   *  Use currently unclear. May be removed later if not needed
   */
  virtual void close_mo() = 0; // unclear

  /**
   * @provide json object for MO content serialization

   * This shall return a JSON object suitable for use in serialization according to the format
   * described in OMA-DM Protocol Spec V2.0 section 7.2.1.4 - management object serialization.
   * The output of this method shall correspond to the "MOData" object in that specification.
   */
  virtual nlohmann::json serialize_json() const = 0;

  /**
   * @}
   */
};

} // namespace
} // namespace

#endif
