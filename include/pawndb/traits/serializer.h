#ifndef PAWNDB_TRAITS_SERIALIZER_H
#define PAWNDB_TRAITS_SERIALIZER_H

#include "pawndb/result.h"
#include "pawndb/types/buffer_table.h"

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
  /** @brief Serialize operation result type */
  using serial_r = Result<std::size_t, SerializerError>;

  /**
   * @brief Serialize value to buffer
   * @param _buffer Target buffer
   * @param _offset Buffer write position
   * @return Bytes write to the buffer
   */
  serial_r serialize(BufferRef _buffer, std::size_t _offset) const noexcept {
    return static_cast<const Derived*>(this)->trait_serialize(_buffer, _offset);
  }

  /**
   * @brief Deserialize value from buffer
   * @param _buffer Source buffer
   * @param _offset Buffer read position
   * @return SerializeR Success: bytes read, Error: code
   */
  serial_r deserialize(const BufferRef _buffer, std::size_t _offset) noexcept {
    return static_cast<Derived*>(this)->trait_deserialize(_buffer, _offset);
  }

 protected:
  SerializerTrait() = default;
  ~SerializerTrait() = default;
};

}  // namespace PawnDB

#endif  // PAWNDB_TRAITS_SERIALIZER_H
