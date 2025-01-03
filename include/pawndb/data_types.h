/**
 * @file data_types.h
 * @author Xiahua Liu @xiahualiu
 * @brief PawnDB is strong typed database, define the data types here.
 * @version 0.1
 * @date 2025-01-02
 *
 * @copyright MIT License
 *
 */

#ifndef PAWNDB_DATA_TYPES_H
#define PAWNDB_DATA_TYPES_H

#include <array>

#include "pawndb/params.h"

namespace PawnDB {

template <typename T>
constexpr d_id_t data_id() = delete;

template <>
constexpr d_id_t data_id<std::uint8_t>() {
  return 1;
}

template <>
constexpr d_id_t data_id<std::int8_t>() {
  return 2;
}

template <>
constexpr d_id_t data_id<std::uint16_t>() {
  return 3;
}

template <>
constexpr d_id_t data_id<std::int16_t>() {
  return 4;
}

template <>
constexpr d_id_t data_id<std::uint32_t>() {
  return 5;
}

template <>
constexpr d_id_t data_id<std::int32_t>() {
  return 6;
}

template <>
constexpr d_id_t data_id<std::uint64_t>() {
  return 7;
}

template <>
constexpr d_id_t data_id<std::int64_t>() {
  return 8;
}

using student_name = std::array<char, 32>;

template <>
constexpr d_id_t data_id<student_name>() {
  return 9;
}

}  // namespace PawnDB

#endif  // PAWNDB_DATA_TYPES_H
