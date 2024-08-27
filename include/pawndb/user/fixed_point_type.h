#ifndef FIXED_POINT_TYPE_H
#define FIXED_POINT_TYPE_H

#include <cstddef>
#include <cstdint>

namespace PawnDB {

  /**
   * @brief A example fixed point class, POD compatible
   * - It has trivial destructor
   * - It has trivial copy constructor
   * - It has trivial copy assignment constructor
   *
   * sign: 1 means positive, 0 means negative.
   * n_part: value of the Numerator part, i.e. the top part of a fraction.
   *
   * @tparam denom The denomerator of the fraction. I.e. 100 means value=n_part/100.
   */
  template <std::uint32_t denom>
  class FixedPoint {
  public:
    inline FixedPoint() : sign(0), n_part(0) { checksum = check_sum(); };
    inline FixedPoint(bool&& is_pos, std::uint32_t&& value) : sign(is_pos), n_part(value) {
      checksum = check_sum();
    };

    // Required checksum function
    inline std::uint32_t check_sum() const { return (sign) ? n_part : ~n_part; }

    // Required hash function
    inline std::size_t hash() const { return (sign) ? n_part : ~n_part; }

    // Required equal function
    bool operator==(const FixedPoint<denom>& f) const {
      if (n_part == 0 && f.n_part == 0)
        return true;
      else
        return this->sign == f.sign && this->n_part == f.n_part;
    }

    inline bool verify_checksum() const { return checksum == check_sum(); }

    bool inline is_pos() const { return sign == 1; }

    // Required not equal function
    bool operator!=(const FixedPoint<denom>& f) const {
      if (n_part == 0 && f.n_part == 0)
        return false;
      else
        return this->sign != f.sign || this->n_part != f.n_part;
    }

    bool operator>(const FixedPoint<denom>& f) const {
      if (n_part == 0 && f.n_part == 0)
        return false;
      else if (sign == 1 && f.sign == 0)
        return true;
      else if (sign == 0 && f.sign == 1)
        return false;
      else if (sign == 1 && n_part > f.n_part)
        return true;
      else if (sign == 0 && n_part < f.n_part)
        return true;
      else
        return false;
    }

    bool operator<(const FixedPoint<denom>& f) const {
      if (n_part == 0 && f.n_part == 0)
        return false;
      else if (sign == 0 && f.sign == 1)
        return true;
      else if (sign == 1 && f.sign == 0)
        return false;
      else if (sign == 1 && n_part < f.n_part)
        return true;
      else if (sign == 0 && n_part > f.n_part)
        return true;
      else
        return false;
    }

  private:
    std::uint8_t sign;
    std::uint32_t n_part;
    volatile std::uint32_t checksum;
  };

}  // namespace PawnDB

#endif