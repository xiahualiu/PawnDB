#include "pawndb/fixed_point.h"

using namespace PawnDB;

FixedPoint::FixedPoint() : sign(0), l_part(0), r_part(0) {}

FixedPoint::FixedPoint(bool&& is_neg, std::uint16_t&& left, std::uint16_t&& right)
    : sign(is_neg), l_part(left), r_part(right) {
  update_check_sum();
}