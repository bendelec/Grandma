/** ***************************************************************************
 * Class MO::StaticData
 * 
 * (c)2020 Christian Bendele
 *
 * See class description in header file
 *
 */

#include "MO_StaticData.h"

namespace Grandma {
namespace MO {

StaticData::StaticData(std::string mo_name, std::string ddf_filename) : BaseCached(ddf_filename) {
  root.uri = mo_name;
}

/**
 *@{
 * See documentation of MO::Interface for the following methods that are all
 * part of the interface
 */
void StaticData::init_mo() {
}

void StaticData::close_mo() {}

std::string StaticData::miid() 
const {
  return "miid1";
}

std::string StaticData::get_val(const std::string node_path, bool&, bool&) {
  return local_get_node(node_path);
}

bool StaticData::set_val(const std::string node_path, const std::string data) {
  local_set_node(node_path, data, true);
  return true;;
}

bool StaticData::remove_node(const std::string node_path) {
  (void)node_path;
  return false;
}

bool StaticData::execute(const std::string node_path) {
  (void)node_path;
  return false;
}

bool StaticData::check_ddf_name_compatibility(std::string ddfname) {
  (void)ddfname;
  // while this class is supposed to support all valid ddf files, it should
  // still check if the ddf file it is being registered against is the same that
  // was used in generating its underlying BaseCached structure.
  return true;	//FIXME
}

/**
 * @}
 */

} // namespace
} // namespace
