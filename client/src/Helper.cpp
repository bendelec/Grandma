/** ***************************************************************************
 * Various helper functions
 *
 * (c)2020 Christian Bendele
 * 
 */

#include "Helper.h"

#include <sstream>

namespace Grandma {

std::vector<std::string> Helper::vectorize_path(const std::string path) {
  std::stringstream ss;

  if(path[0] == '/') {
    ss << path.substr(1);
  } else {
    ss << path;
  }

  std::string segment;
  std::vector<std::string> vectorized;

  while(getline(ss, segment, '/')) {
    vectorized.push_back(segment);
  }
  return vectorized;
}

} // namespace
