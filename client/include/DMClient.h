/**
 * Composite OMADM client class for Grandma
 *
 * (c) 2020 Christian Bendele
 *
 * This class pulls together the various sub components that compose
 * the OMADM client (MOTree, Session, Alertqueue, CommandQueue) and 
 * manages collaboration between them.
 *
 * It is also the implementation class of the main omadm-client interface
 * (which will later be provided as a pimpl idiom to provide a cleaner interface
 * and compile barrier from all the internal dependencies
 *
 * TODO: This is very much work in progress. Don't rely on the interface to be stable
 * yet.
 *
 * TODO: The CommandQueue class is currently missing, and the functionality is mixed up
 * between this class and the Session class. This needs a clean architecture and then
 * refactoring
 *
 */
#ifndef GRANDMA_DMCLIENT_H
#define GRANDMA_DMCLIENT_H

#include <string>

#include "MOTree.h"
#include "AlertQueue.h"
#include "CommandQueue.h"
#include "Session.h"

namespace Grandma {

class DMClient {
  std::string DevId;    // TODO: duplicate data with DevInfo MO.... remove once proper handling of DevInfo mandatory MO is finished

  MOTree        motree;
  CommandQueue  command_queue;
  Session       session;
  AlertQueue    alert_queue;

  bool  P1_dump_tree; // See comment on set_P1_dump_tree (in source file)

public:

  DMClient();

  void register_DDF(std::string urn, std::string filename, std::string ddf_url = "");
  void add_MO(std::string urn, std::shared_ptr<MO::Interface> mo, std::string miid = "");

  void start_session(bool server_initiated = false);

  void set_P1_dump_tree(bool enable = true);

  void set_device_id(std::string id);

  void finish_bootstrap();


};

} //namespace

#endif
