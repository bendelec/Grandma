/** ***************************************************************************
 * Various helper functions
 *
 * (c)2020 Christian Bendele
 * 
 * This (static) class is a collection of helper function that are useful
 * in various different places inside the library but might also be useful
 * for local applications using the library, and are therefor part of the
 * interface.
 */
#ifndef GRANDMA_HELPER_H
#define GRANDMA_HELPER_H

#include <vector>
#include <string>
#include <sstream>

namespace Grandma {

class Helper {

public:
  /**
   * @brief Helper function to split a path string into a vector of its segments
   * 
   * @param[in] path - For example "Foo/Bar/Baz"
   * @return In this example a vector with the elements {"Foo", "Bar", "Baz"}
   */
  static std::vector<std::string> vectorize_path(const std::string path);

};

} // namespace

#endif
