#ifndef U32_TYPE_H
#define U32_TYPE_H

#include <cstddef>
#include <cstdint>

#include "pawndb/base_type.h"

namespace PawnDB {

  /**
   * @brief A example u32 type function
   *
   */
  class U32Int : public Base_Type<std::uint32_t> {
  public:
    U32Int();

    // Required checksum function
    inline std::uint32_t check_sum() const override { return value; }

    // Required hash function
    inline std::size_t hash() const override { return value; }

    // Required == function
    bool operator==(const U32Int& i) const {
      return value == i.value;
    }

    // Required != function
    bool operator!=(const U32Int& i) const {
      return value != i.value;
    }

    std::uint32_t value;

  private:
  };

}  // namespace PawnDB

#endif