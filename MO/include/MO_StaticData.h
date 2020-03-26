/**
 * Static Data repository MO
 *
 * (c)2020 Christian Bendele
 *
 * This class is an absolute minimum implementation of a complete (as in non-abstract, instanciable)
 * managed object (MO) class for the Grandma OMA-DM client library.
 *
 * This MO type supports a passive data store. The node tree of key/value nodes can be created
 * from a ddf file (inherited from BaseCached), and the nodes can be read and updated
 * from both the backend side and the local application side.
 * 
 * No execution of nodes is supported, nor any triggering of actions (via callbacks etc)
 * on updates to nodes
 *
 */
#ifndef GRANDMA_MO_STATICDATA
#define GRANDMA_MO_STATICDATA

#include <string>
#include <vector>

#include "MO_BaseCached.h"

namespace Grandma {
namespace MO {

class StaticData : public BaseCached {

public:
  StaticData(std::string mo_name, std::string ddf_filename);

  virtual void init_mo(); // gets called by the client on adding an instance to the tree.
  virtual void close_mo(); // unclear

  virtual std::string miid() const;

  virtual std::string get_val(const std::string node_path, bool&, bool&);
  virtual bool set_val(const std::string node_path, const std::string raw_data);
  virtual bool remove_node(const std::string node_path);
  virtual bool execute(const std::string node_path);

  virtual bool check_ddf_name_compatibility(std::string ddfname);
};


} // namespace
} // namespace

#endif
