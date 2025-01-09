#ifndef PAWNDB_TRAITS_SERIALIZER_H
#define PAWNDB_TRAITS_SERIALIZER_H

#include "pawndb/result.h"

/**
 * @file serializer.h
 * @brief CRTP interface for data serialization
 * @version 0.1
 * @date 2025-01-02
 *
 * Features:
 * - Error handling
 * - Size calculations
 */

namespace PawnDB {
/**
 * @brief Serialization operation error codes
 */
enum class SerializerError {
  None,           /**< Operation successful */
  TypeMismatch,   /**< Type verification failed */
  CheckSumFailed, /**< Checksum verification failed */
};

/**
 * @brief CRTP interface for serializer implementations
 * @tparam Derived Class implementing serializer interface
 *
 * Required implementations:
 * - SerializeR trait_serialize(const value_type&, buffer_type&, size_type)
 * - SerializeR trait_deserialize(const buffer_type&, size_type, value_type&)
 * - size_type trait_size(const value_type&)
 */
template <typename Derived>
class SerializerTrait {
 public:
  using SerializeR = Result<std::size_t, SerializerError>;

  /**
   * @brief Serialize value to buffer
   * @param buffer Target buffer
   * @param offset Buffer write position
   * @return Bytes write to the buffer
   */
  std::size_t serialize(char* buffer, std::size_t offset) const noexcept {
    return static_cast<const Derived*>(this)->trait_serialize(buffer, offset);
  }

  /**
   * @brief Deserialize value from buffer
   * @param buffer Source buffer
   * @param offset Buffer read position
   * @return SerializeR Success: bytes read, Error: code
   */
  SerializeR deserialize(const char* buffer, std::size_t offset) noexcept {
    return static_cast<Derived*>(this)->trait_deserialize(buffer, offset);
  }

 protected:
  SerializerTrait() = default;
  ~SerializerTrait() = default;
};

}  // namespace PawnDB

#endif  // PAWNDB_TRAITS_SERIALIZER_H
