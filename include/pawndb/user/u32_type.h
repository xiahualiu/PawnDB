#ifndef U32_TYPE_H
#define U32_TYPE_H

#include <cstddef>
#include <cstdint>

namespace PawnDB {

  /**
   * @brief A example u32 type class, POD compatible.
   * - It has trivial destructor
   * - It has trivial copy constructor
   * - It has trivial copy assignment constructor
   */
  class U32Int {
  public:
    inline U32Int() : value(0u) { checksum = check_sum(); };
    inline U32Int(const std::uint32_t v) : value(v) { checksum = check_sum(); };

    // Required checksum function
    inline std::uint32_t check_sum() const { return value; }

    // Required hash function
    inline std::size_t hash() const { return value; }

    // Required == function
    bool operator==(const U32Int& i) const { return value == i.value; }
    // Required != function
    bool operator!=(const U32Int& i) const { return value != i.value; }
    // Required == function
    bool operator>(const U32Int& i) const { return value > i.value; }
    // Required != function
    bool operator<(const U32Int& i) const { return value < i.value; }

    // Required verify function
    inline bool verify_checksum() const { return checksum == check_sum(); }

    std::uint32_t value;

  private:
    volatile std::uint32_t checksum;
  };

}  // namespace PawnDB

#endif