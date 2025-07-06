/**
 * @file parser.h
 * @author Xiahua Liu @xiahualiu
 * @brief PawnDB API message parser based on serializer specification.
 * @version 0.1
 * @date 2025-01-06
 *
 * @copyright MIT License
 *
 */

#ifndef PAWNDB_TYPES_PARSER_H
#define PAWNDB_TYPES_PARSER_H

#include <cstddef>
#include <cstring>

#include "pawndb/params.h"
#include "pawndb/result.h"

namespace PawnDB {

/**
 * @brief Parser error codes
 */
enum class ParserError {
  None,             /**< No error */
  ReadAfterEnd,     /**< Read past end of input */
  InvalidValue,     /**< Invalid value encountered */
  InvalidOperation, /**< Invalid operation type */
  InvalidSize,      /**< Invalid buffer size */
  ChecksumError     /**< Checksum validation failed */
};

/**
 * @brief Operation types based on serializer specification
 */
enum class OpType : std::uint8_t {
  // Transaction Management
  START_TRANSACTION = 0x01,
  COMMIT_TRANSACTION = 0x02,
  ROLLBACK_TRANSACTION = 0x03,

  // Blocking Lock Operations
  WAIT_ANY_RECORD_SHARED_LOCK = 0x10,
  WAIT_ANY_RECORD_EXCLUSIVE_LOCK = 0x11,
  WAIT_RECORD_SHARED_LOCK = 0x12,
  WAIT_RECORD_EXCLUSIVE_LOCK = 0x13,
  WAIT_ALL_RECORDS_SHARED_LOCK = 0x14,
  WAIT_ALL_RECORDS_EXCLUSIVE_LOCK = 0x15,

  // Non-blocking Lock Operations
  GET_RECORD_SHARED_LOCK = 0x20,
  GET_RECORD_EXCLUSIVE_LOCK = 0x21,
  GET_ALL_RECORDS_SHARED_LOCK = 0x22,
  GET_ALL_RECORDS_EXCLUSIVE_LOCK = 0x23,

  // Read Operations
  READ_TUPLE_WITH_KEY = 0x30,

  // Lock Management
  YIELD_SHARED_LOCK = 0x40,
  PROMOTE_SHARED_LOCK = 0x41,

  // Insert Operations
  INSERT_RECORD_WITH_KEY = 0x50,
  INSERT_RECORD_AUTO_KEY = 0x51,

  // Update Operations
  UPDATE_RECORD_WITH_KEY = 0x60,

  // Delete Operations
  DELETE_RECORD_WITH_KEY = 0x70,

  // Maximum value for operation type
  MAX_OP_VALUE = 0x7F
};

/**
 * @brief Response error codes
 */
enum class ResponseError : std::uint8_t {
  NO_ERROR = 0x00,
  TIMEOUT = 0x01,
  RECORD_NOT_FOUND = 0x02,
  RECORD_NOT_LOCKED = 0x03,
  RECORD_UNAVAILABLE = 0x04,
  DUPLICATE_KEY = 0x05,
  TABLE_FULL = 0x06,
  INVALID_TRANSACTION = 0x07,
  INVALID_OPERATION = 0x08,
  SERIALIZATION_ERROR = 0x09,
  CHECKSUM_ERROR = 0x0A,
  VERSION_MISMATCH = 0x0B
};

/**
 * @brief PawnDB API message parser
 *
 * Parses binary messages according to the PawnDB API specification.
 * All messages follow the format:
 * - API_Version (4 bytes, uint32_t)
 * - Message_Length (2 bytes, uint16_t)
 * - Op_Type (1 byte, uint8_t)
 * - Additional fields based on operation type
 * - Check_Sum (4 bytes, uint32_t) at the end
 */
class Parser {
 public:
  // Result type aliases
  using op_r = Result<OpType, ParserError>;
  using txn_id_r = Result<txn_id_t, ParserError>;
  using response_error_r = Result<ResponseError, ParserError>;
  using table_id_r = Result<std::uint32_t, ParserError>;
  using tuple_key_r = Result<std::uint32_t, ParserError>;
  using api_version_r = Result<std::uint32_t, ParserError>;
  using message_length_r = Result<std::uint16_t, ParserError>;
  using checksum_r = Result<std::uint32_t, ParserError>;
  using serializer_version_r = Result<std::uint32_t, ParserError>;
  using tuple_data_length_r = Result<std::uint16_t, ParserError>;
  using tuple_data = Result<const char*, ParserError>;

  /**
   * @brief Construct parser with buffer reference
   * @param _buffer Buffer containing message data
   * @param _size Size of message data in buffer
   */
  Parser(buffer_t& _buffer, std::size_t _size) noexcept
      : buffer_(_buffer), size_(_size) {}

  // Non-copyable
  Parser(const Parser& _other) = delete;
  Parser& operator=(const Parser& _other) = delete;

  // Common message field getters

  /**
   * @brief Get API version from message
   * @return Result containing API version or error
   */
  api_version_r get_api_version() const noexcept {
    if (size_ < 4) return ParserError::ReadAfterEnd;
    std::uint32_t version;
    std::memcpy(&version, &buffer_[0], sizeof(std::uint32_t));
    return version;
  }

  /**
   * @brief Get message length from message
   * @return Result containing message length or error
   */
  message_length_r get_message_length() const noexcept {
    if (size_ < 6) return ParserError::ReadAfterEnd;
    std::uint16_t length;
    std::memcpy(&length, &buffer_[4], sizeof(std::uint16_t));
    return length;
  }

  /**
   * @brief Get operation type from message
   * @return Result containing operation type or error
   */
  op_r get_op_type() const noexcept {
    if (size_ < 7) return ParserError::ReadAfterEnd;
    auto op = static_cast<OpType>(buffer_[6]);
    return op;
  }

  /**
   * @brief Get transaction ID from message
   * @return Result containing transaction ID or error
   */
  txn_id_r get_transaction_id() const noexcept {
    if (size_ < 11) return ParserError::ReadAfterEnd;
    txn_id_t txn_id;
    std::memcpy(&txn_id, &buffer_[7], sizeof(txn_id_t));
    return txn_id;
  }

  /**
   * @brief Get response error from message
   * @return Result containing response error or error
   */
  response_error_r get_response_error() const noexcept {
    if (size_ < 12) return ParserError::ReadAfterEnd;
    auto error = static_cast<ResponseError>(buffer_[11]);
    return error;
  }

  /**
   * @brief Get table ID from message
   * @return Result containing table ID or error
   */
  table_id_r get_table_id() const noexcept {
    if (size_ < 16) return ParserError::ReadAfterEnd;
    std::uint32_t table_id;
    std::memcpy(&table_id, &buffer_[12], sizeof(std::uint32_t));
    return table_id;
  }

  /**
   * @brief Get tuple key from message
   * @return Result containing tuple key or error
   */
  tuple_key_r get_tuple_key() const noexcept {
    if (size_ < 20) return ParserError::ReadAfterEnd;
    std::uint32_t key;
    std::memcpy(&key, &buffer_[16], sizeof(std::uint32_t));
    return key;
  }

  /**
   * @brief Get tuple serializer version from message
   * @return Result containing serializer version or error
   */
  serializer_version_r get_tuple_serializer_version() const noexcept {
    if (size_ < 24) return ParserError::ReadAfterEnd;
    std::uint32_t version;
    std::memcpy(&version, &buffer_[20], sizeof(std::uint32_t));
    return version;
  }

  /**
   * @brief Get tuple data length from message
   * @return Result containing data length or error
   */
  tuple_data_length_r get_tuple_data_length() const noexcept {
    if (size_ < 26) return ParserError::ReadAfterEnd;
    std::uint16_t length;
    std::memcpy(&length, &buffer_[24], sizeof(std::uint16_t));
    return length;
  }

  /**
   * @brief Get pointer to tuple data in message
   * @return Pointer to tuple data or nullptr if not available
   */
  const char* get_tuple_data() const noexcept {
    if (size_ < 26) return nullptr;
    return &buffer_[26];
  }

  /**
   * @brief Get tuple key list length from message
   * @return Result containing key list length or error
   */
  tuple_data_length_r get_tuple_key_length() const noexcept {
    if (size_ < 22) return ParserError::ReadAfterEnd;
    std::uint16_t length;
    std::memcpy(&length, &buffer_[20], sizeof(std::uint16_t));
    return length;
  }

  /**
   * @brief Get pointer to tuple key list in message
   * @return Pointer to tuple key list or nullptr if not available
   */
  const std::uint32_t* get_tuple_keys() const noexcept {
    if (size_ < 22) return nullptr;
    return reinterpret_cast<const std::uint32_t*>(&buffer_[22]);
  }

  /**
   * @brief Get checksum from message
   * @return Result containing checksum or error
   */
  checksum_r get_checksum() const noexcept {
    if (size_ < 4) return ParserError::ReadAfterEnd;
    std::uint32_t checksum;
    std::memcpy(&checksum, &buffer_[size_ - 4], sizeof(std::uint32_t));
    return checksum;
  }

  // Common message field setters

  /**
   * @brief Set API version in message
   * @param version API version to set
   */
  void set_api_version(std::uint32_t version) noexcept {
    std::memcpy(&buffer_[0], &version, sizeof(std::uint32_t));
  }

  /**
   * @brief Set message length in message
   * @param length Message length to set
   */
  void set_message_length(std::uint16_t length) noexcept {
    std::memcpy(&buffer_[4], &length, sizeof(std::uint16_t));
  }

  /**
   * @brief Set operation type in message
   * @param op Operation type to set
   */
  void set_op_type(OpType op) noexcept {
    buffer_[6] = static_cast<char>(op);
  }

  /**
   * @brief Set transaction ID in message
   * @param txn_id Transaction ID to set
   */
  void set_transaction_id(txn_id_t txn_id) noexcept {
    std::memcpy(&buffer_[7], &txn_id, sizeof(txn_id_t));
  }

  /**
   * @brief Set response error in message
   * @param error Response error to set
   */
  void set_response_error(ResponseError error) noexcept {
    buffer_[11] = static_cast<char>(error);
  }

  /**
   * @brief Set table ID in message
   * @param table_id Table ID to set
   */
  void set_table_id(std::uint32_t table_id) noexcept {
    std::memcpy(&buffer_[12], &table_id, sizeof(std::uint32_t));
  }

  /**
   * @brief Set tuple key in message
   * @param key Tuple key to set
   */
  void set_tuple_key(std::uint32_t key) noexcept {
    std::memcpy(&buffer_[16], &key, sizeof(std::uint32_t));
  }

  /**
   * @brief Set tuple serializer version in message
   * @param version Serializer version to set
   */
  void set_tuple_serializer_version(std::uint32_t version) noexcept {
    std::memcpy(&buffer_[20], &version, sizeof(std::uint32_t));
  }

  /**
   * @brief Set tuple data length in message
   * @param length Data length to set
   */
  void set_tuple_data_length(std::uint16_t length) noexcept {
    std::memcpy(&buffer_[24], &length, sizeof(std::uint16_t));
  }

  /**
   * @brief Set tuple data in message
   * @param data Pointer to tuple data
   * @param length Length of tuple data
   */
  void set_tuple_data(const char* data, std::size_t length) noexcept {
    if (length + 26 + 4 <= BUFFER_WIDTH) {
      std::memcpy(&buffer_[26], data, length);
      size_ = 26 + length;  // Update size to include tuple data
    }
  }

  /**
   * @brief Set tuple key list length in message
   * @param length Key list length to set
   */
  ParserError set_tuple_key_length(std::uint16_t length) noexcept {
    if (length * sizeof(std::uint32_t) + 22 + 4 <= BUFFER_WIDTH) {
      std::memcpy(&buffer_[20], &length, sizeof(std::uint16_t));
      return ParserError::None;
    } else {
      return ParserError::InvalidSize;
    }
  }

  /**
   * @brief Set tuple keys in message
   * @param keys Pointer to tuple keys
   * @param count Number of keys
   */
  ParserError set_tuple_keys(const std::uint32_t* keys,
                             std::size_t count) noexcept {
    if (count * sizeof(std::uint32_t) + 22 + 4 <= BUFFER_WIDTH) {
      std::memcpy(&buffer_[22], keys, count * sizeof(std::uint32_t));
      return ParserError::None;
    } else {
      return ParserError::InvalidSize;
    }
  }

  /**
   * @brief Validate message checksum
   * @return True if checksum is valid
   */
  bool validate_checksum() const noexcept {
    auto checksum_result = get_checksum();
    if (!checksum_result) return false;

    return checksum_result.unwrap() == calculate_checksum();
  }

  /**
   * @brief Finalize message by updating length and checksum
   * @return ParserError indicating success or failure
   */
  ParserError finalize_message() noexcept {
    // Update message length
    auto length_result = update_response_message_size();
    if (length_result != ParserError::None) return length_result;
    // Update checksum
    set_checksum(calculate_checksum());
    return ParserError::None;
  }

 private:
  /**
   * @brief Set checksum in message
   * @param checksum Checksum to set
   */
  void set_checksum(std::uint32_t checksum) noexcept {
    std::memcpy(&buffer_[size_ - 4], &checksum, sizeof(std::uint32_t));
  }

  ParserError update_response_message_size() noexcept {
    // Read operation type and calculate size
    auto op = get_op_type();
    if (!op) return ParserError::InvalidOperation;
    std::uint16_t op_size =
        7;  // Base size for API version, message length, and op type
    switch (op.unwrap()) {
      case OpType::START_TRANSACTION:
      case OpType::COMMIT_TRANSACTION:
      case OpType::ROLLBACK_TRANSACTION:
        // Transaction ID(4) + Response(1) + checksum(4)
        op_size += 9;
        break;
      case OpType::WAIT_ANY_RECORD_SHARED_LOCK:
      case OpType::WAIT_ANY_RECORD_EXCLUSIVE_LOCK:
      case OpType::WAIT_RECORD_SHARED_LOCK:
      case OpType::WAIT_RECORD_EXCLUSIVE_LOCK:
        // Transaction ID(4) + Response(1) + Table ID(4) + Tuple Key(4) +
        // Checksum(4)
        op_size += 17;
        break;
      case OpType::WAIT_ALL_RECORDS_SHARED_LOCK:
      case OpType::WAIT_ALL_RECORDS_EXCLUSIVE_LOCK:
        // Transaction ID(4) + Response(1) + Table ID(4) + Tuple Key Length(2) +
        // checksum(4)
        op_size += 15;
        op_size += sizeof(tp_id_t) * get_tuple_key_length().unwrap();
        break;
      case OpType::GET_RECORD_SHARED_LOCK:
      case OpType::GET_RECORD_EXCLUSIVE_LOCK:
        // Transaction ID(4) + Response(1) + Table ID(4) + Tuple Key(4) +
        // Checksum(4)
        op_size += 17;
        break;
      case OpType::GET_ALL_RECORDS_SHARED_LOCK:
      case OpType::GET_ALL_RECORDS_EXCLUSIVE_LOCK:
        // Transaction ID(4) + Response(1) + Table ID(4) + Tuple Key Length(2) +
        // checksum(4)
        op_size += 15;
        op_size += sizeof(tp_id_t) * get_tuple_key_length().unwrap();
        break;
      case OpType::READ_TUPLE_WITH_KEY:
        // Transaction ID(4) + Response(1) + Table ID(4) + Tuple Key(4) + Tuple
        // Serializer Version(4) + Tuple Data Length(2)+Checksum(4)
        op_size += 23;
        op_size += get_tuple_data_length().unwrap();
        break;
      case OpType::YIELD_SHARED_LOCK:
      case OpType::PROMOTE_SHARED_LOCK:
      case OpType::INSERT_RECORD_WITH_KEY:
      case OpType::INSERT_RECORD_AUTO_KEY:
      case OpType::UPDATE_RECORD_WITH_KEY:
      case OpType::DELETE_RECORD_WITH_KEY:
        // Transaction ID(4) + Response(1) + Table ID(4) + Tuple Key(4) +
        op_size += 17;
        break;
      default:
        return ParserError::InvalidOperation;  // Unsupported operation type
    }
    // Set the new message length
    set_message_length(op_size);
    size_ = op_size;
    return ParserError::None;
  }

  /**
   * @brief Calculate CRC32 checksum for message content
   * @return Calculated checksum
   */
  std::uint32_t calculate_checksum() const noexcept {
    // Simple CRC32 implementation for message validation
    std::uint32_t crc = 0xFFFFFFFF;
    for (std::size_t i = 0; i < size_ - 4; ++i) {
      crc ^= static_cast<std::uint32_t>(buffer_[i]);
      for (int j = 0; j < 8; ++j) {
        if (crc & 1) {
          crc = (crc >> 1) ^ 0xEDB88320;
        } else {
          crc >>= 1;
        }
      }
    }
    return crc ^ 0xFFFFFFFF;
  }

  buffer_t& buffer_; /**< Buffer containing message data */
  std::size_t size_; /**< Size of message data */
};

}  // namespace PawnDB

#endif  // PAWNDB_TYPES_PARSER_H
