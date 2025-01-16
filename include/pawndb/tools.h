#ifndef PAWNDB_TOOLS_H
#define PAWNDB_TOOLS_H

#include <array>
#include <cstddef>

namespace PawnDB {

/** @brief Compare two arrays
 *  @param a First array
 *  @param b Second array
 *  @return -1 if a < b, 0 if a == b, 1 if a > b
 */
template <std::size_t N>
int array_cmp(const std::array<char, N>& a,
              const std::array<char, N>& b) noexcept {
  for (std::size_t i = 0; i < N; i++) {
    if (a[i] != b[i]) {
      return (a[i] < b[i]) ? -1 : 1;
    }
    if (a[i] == '\0') {
      return 0;  // Equal up to null terminator
    }
  }
  return 0;  // Arrays are equal
}


}  // namespace PawnDB
#endif
