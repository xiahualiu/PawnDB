#ifndef FIXED_POINT_H
#define FIXED_POINT_H

#include <cstdint>

#include "pawndb/base_type.h"

namespace PawnDB {

  /**
   * @brief A example fixed point class
   *
   * sign: 0 means positive, 1 means negative.
   * l_part: value of the left part, aka > 1.0 part
   * r_part: value of the right part, aka < 1.0 part
   *
   */
  class FixedPoint : public Base_Type {
  public:
    FixedPoint();
    FixedPoint(bool&& is_neg, std::uint16_t&& left, std::uint16_t&& right);

    inline std::uint32_t check_sum() const override { return r_part + (l_part << 16) + sign; }

    inline void update_check_sum() { this->checksum = check_sum(); }

    std::uint8_t sign;
    std::uint16_t l_part;
    std::uint16_t r_part;

  private:
  };

}  // namespace PawnDB

#endif