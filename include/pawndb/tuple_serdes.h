/**
 * @file tuple_serdes.h
 * @author Xiahua Liu @xiahualiu
 * @brief Tuple serialization and deserialization.
 * @version 0.1
 * @date 2025-01-02
 *
 * @copyright MIT License
 *
 */

#ifndef PAWNDB_TP_SERDES_H
#define PAWNDB_TP_SERDES_H

#include <cstring>

#include "pawndb/buffer_table.h"
#include "pawndb/data_types.h"
#include "pawndb/params.h"
#include "pawndb/result.h"
#include "pawndb/table.h"

namespace PawnDB {

/**
 * @brief Enumeration of possible deserialization errors.
 */
enum class DeserialError {
  None,      /**< No error */
  IDMismatch /**< ID mismatch error */
};

/**
 * @brief Template class for tuple serialization and deserialization.
 *
 * @tparam T The type of the tuple.
 */
template <typename T>
class TpSerDes;

/**
 * @brief Specialization of TpSerDes for Table.
 *
 * @tparam Rows The number of rows in the table.
 * @tparam Ts The types of the columns in the table.
 */
template <tbl_row_t Rows, typename... Ts>
class TpSerDes<Table<Rows, Ts...>> {
 public:
  /**
   * @brief Serializes a tuple into a buffer.
   *
   * @param _tuple The tuple to serialize.
   * @param _buffer The buffer to serialize into.
   * @param _offset The offset in the buffer to start serialization.
   * @return The new offset after serialization.
   */
  static buf_size_t serialize(const std::tuple<Ts...>& _tuple,
                              BufferTable::buffer_t& _buffer,
                              buf_size_t _offset) noexcept {
    return serialize_impl(_tuple, _buffer, _offset);
  }

  using DeSerialR = Result<buf_size_t, DeserialError>;
  /**
   * @brief Deserializes a tuple from a buffer.
   *
   * @param _buffer The buffer to deserialize from.
   * @param _offset The offset in the buffer to start deserialization.
   * @param _tuple The tuple to deserialize into.
   * @return The new offset after deserialization, or an error if
   * deserialization fails.
   */
  static DeSerialR deserialize(const BufferTable::buffer_t& _buffer,
                               buf_size_t _offset,
                               std::tuple<Ts...>& _tuple) noexcept {
    return deserialize_impl(_buffer, _offset, _tuple);
  }

 private:
  /**
   * @brief Helper function for tuple serialization.
   *
   * @tparam I The index of the tuple element to serialize.
   * @param _tuple The tuple to serialize.
   * @param _buffer The buffer to serialize into.
   * @param _offset The offset in the buffer to start serialization.
   * @return The new offset after serialization.
   */
  template <std::size_t I = 0>
  static buf_size_t serialize_impl(const std::tuple<Ts...>& _tuple,
                                   BufferTable::buffer_t& _buffer,
                                   buf_size_t& _offset) noexcept {
    if constexpr (I == sizeof...(Ts)) {
      return _offset;
    } else {
      using CurrentType = std::tuple_element_t<I, std::tuple<Ts...>>;
      d_id_t id = data_id<CurrentType>();
      std::memcpy(_buffer.data() + _offset, &id, sizeof(d_id_t));
      _offset += sizeof(d_id_t);
      const auto& data = std::get<I>(_tuple);
      std::memcpy(_buffer.data() + _offset, &data, sizeof(CurrentType));
      _offset += sizeof(CurrentType);
      return serialize_impl<I + 1>(_tuple, _buffer, _offset);
    }
  }

  /**
   * @brief Helper function for tuple deserialization.
   *
   * @tparam I The index of the tuple element to deserialize.
   * @param _buffer The buffer to deserialize from.
   * @param _offset The offset in the buffer to start deserialization.
   * @param _tuple The tuple to deserialize into.
   * @return The new offset after deserialization, or an error if
   * deserialization fails.
   */
  template <std::size_t I = 0>
  static DeSerialR deserialize_impl(const BufferTable::buffer_t& _buffer,
                                    buf_size_t& _offset,
                                    std::tuple<Ts...>& _tuple) noexcept {
    if constexpr (I == sizeof...(Ts)) {
      return _offset;
    } else {
      using CurrentType = std::tuple_element_t<I, std::tuple<Ts...>>;
      d_id_t id;
      std::memcpy(&id, _buffer.data() + _offset, sizeof(d_id_t));
      if (id != data_id<CurrentType>()) {
        return DeSerialR(DeserialError::IDMismatch);
      }
      _offset += sizeof(d_id_t);
      CurrentType data;
      std::memcpy(&data, _buffer.data() + _offset, sizeof(CurrentType));
      _offset += sizeof(CurrentType);
      std::get<I>(_tuple) = data;
      return deserialize_impl<I + 1>(_buffer, _offset, _tuple);
    }
  }
};

}  // namespace PawnDB

#endif  // PAWNDB_TP_SERDES_H
