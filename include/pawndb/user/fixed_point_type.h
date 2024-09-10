#ifndef FIXED_POINT_TYPE_H
#define FIXED_POINT_TYPE_H

#include <pawndb/basic_data.h>

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>

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
  class FixedPoint : public BasicData<FixedPoint<denom>> {
  public:
    FixedPoint() : sign(0), n_part(0) { checksum = check_sum(); };
    FixedPoint(bool&& is_pos, std::uint32_t&& value) : sign(is_pos), n_part(value) {
      checksum = check_sum();
    };

    bool is_pos() const { return sign == 1; }

    std::uint32_t check_sum() const { return (sign) ? n_part : ~n_part; }

    // Required functions
    std::size_t user_hash() const { return (sign) ? n_part : ~n_part; }

    bool user_verify() const { return checksum == check_sum(); }

    std::string user_print() const {
      return (is_pos() ? std::string("+") : std::string("-")) + std::to_string(n_part / denom) + "."
             + to_string_with_zero_padding(n_part % denom);
    }

    std::string to_string_with_zero_padding(const std::size_t& value) const {
      auto str = std::to_string(value);
      if (str.length() < std::log10(denom)) str.insert(0, std::log10(denom) - str.length(), '0');
      return str;
    }

    // Required equal function
    bool user_eq(const FixedPoint<denom>& f) const {
      if (n_part == 0 && f.n_part == 0)
        return true;
      else
        return this->sign == f.sign && this->n_part == f.n_part;
    }

    // Required not equal function
    bool user_neq(const FixedPoint<denom>& f) const {
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