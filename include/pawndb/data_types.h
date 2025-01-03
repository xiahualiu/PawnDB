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

/**
 * @brief Template function to get the data ID for a given type.
 *
 * This function is deleted for all types except the specializations provided
 * below.
 *
 * @tparam T The type for which to get the data ID.
 * @return The data ID for the given type.
 */
template <typename T>
constexpr d_id_t data_id() = delete;

/**
 * @brief Specialization of data_id for std::uint8_t.
 *
 * @return The data ID for std::uint8_t.
 */
template <>
constexpr d_id_t data_id<std::uint8_t>() {
  return 1;
}

/**
 * @brief Specialization of data_id for std::int8_t.
 *
 * @return The data ID for std::int8_t.
 */
template <>
constexpr d_id_t data_id<std::int8_t>() {
  return 2;
}

/**
 * @brief Specialization of data_id for std::uint16_t.
 *
 * @return The data ID for std::uint16_t.
 */
template <>
constexpr d_id_t data_id<std::uint16_t>() {
  return 3;
}

/**
 * @brief Specialization of data_id for std::int16_t.
 *
 * @return The data ID for std::int16_t.
 */
template <>
constexpr d_id_t data_id<std::int16_t>() {
  return 4;
}

/**
 * @brief Specialization of data_id for std::uint32_t.
 *
 * @return The data ID for std::uint32_t.
 */
template <>
constexpr d_id_t data_id<std::uint32_t>() {
  return 5;
}

/**
 * @brief Specialization of data_id for std::int32_t.
 *
 * @return The data ID for std::int32_t.
 */
template <>
constexpr d_id_t data_id<std::int32_t>() {
  return 6;
}

/**
 * @brief Specialization of data_id for std::uint64_t.
 *
 * @return The data ID for std::uint64_t.
 */
template <>
constexpr d_id_t data_id<std::uint64_t>() {
  return 7;
}

/**
 * @brief Specialization of data_id for std::int64_t.
 *
 * @return The data ID for std::int64_t.
 */
template <>
constexpr d_id_t data_id<std::int64_t>() {
  return 8;
}

using student_name = std::array<char, 32>;

/**
 * @brief Specialization of data_id for student_name.
 *
 * @return The data ID for student_name.
 */
template <>
constexpr d_id_t data_id<student_name>() {
  return 9;
}

}  // namespace PawnDB

#endif  // PAWNDB_DATA_TYPES_H
