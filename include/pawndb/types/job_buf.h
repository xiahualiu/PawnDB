/**
 * @file job_buf.h
 * @author Xiahua Liu @xiahualiu
 * @brief PawnDB job buffer — owns a buffer and provides protocol parsing.
 * @version 0.1
 * @date 2025-01-02
 *
 * @copyright MIT License
 *
 */

#ifndef PAWNDB_TYPES_JOB_BUF_H
#define PAWNDB_TYPES_JOB_BUF_H

#include <cstddef>
#include <cstring>
#include <string>

#include "pawndb/params.h"
#include "pawndb/result.h"
#include "pawndb/types/buf_ref.h"

namespace PawnDB {

/** @brief Operation types for database operations */
enum class OpType : op_t {
  START_TXN,      /**< Start a new transaction */
  ADD_TUPLE,      /**< Add a new record */
  SHARED_READ,    /**< Shared read operation */
  EXCLUSIVE_READ, /**< Exclusive read/write operation */
  YIELD_READ,     /**< Release shared lock */
  PROMOTE,        /**< Promote shared lock to exclusive */
  UPDATE,         /**< Update record */
  DELETE,         /**< Remove record */
  COMMIT_TXN,     /**< Commit transaction */
  ABORT_TXN,      /**< Abort transaction */
  MAX_OP_VALUE    /**< Sentinel, must be last */
};

/** @brief Operation acknowledgement types */
enum class OpAck : std::uint8_t {
  SUCCESS,     /**< Operation completed successfully */
  BUSY,        /**< System is busy, retry later */
  BAD_OP,      /**< Invalid operation */
  BAD_TABLE,   /**< Invalid table ID */
  BAD_TP,      /**< Invalid tuple key */
  BAD_TXN,     /**< Invalid transaction ID */
  BAD_PHASE,   /**< Invalid transaction phase */
  BAD_DATA,    /**< Invalid data */
  BAD_ACCESS,  /**< Access violation */
  COMMIT_FULL, /**< Commit table full */
  TIMEOUT,     /**< Operation timed out */
  ABORTED,     /**< Transaction aborted */
  DEAD_TXN     /**< Dead transaction */
};

/** @brief Parser error codes */
enum class ParserError {
  None,         /**< No error */
  ReadAfterEnd, /**< Read beyond buffer end */
  InvalidValue  /**< Invalid value encountered */
};

/** @brief Protocol field offsets for binary message layout */
enum class FieldOffset : std::size_t {
  OP = 0,     /**< Operation type */
  OP_ID = 1,  /**< Operation ID */
  TXN = 2,    /**< Transaction ID (4 bytes) */
  ACK = 6,    /**< Acknowledgement */
  TBL = 7,    /**< Table ID */
  TP_KEY = 8, /**< Tuple key */
  TUPLE = 9,  /**< Tuple data start */
};

/** @brief Parser operation result type */
using op_r = Result<OpType, ParserError>;

/** @brief Parser operation ID result type */
using op_id_r = Result<op_t, ParserError>;

/** @brief Parser transaction ID result type */
using txn_id_r = Result<txn_id_t, ParserError>;

/** @brief Parser acknowledgment result type */
using op_ack_r = Result<OpAck, ParserError>;

/** @brief Parser table ID result type */
using tbl_id_r = Result<tp_id_t, ParserError>;

/** @brief Parser tuple key result type */
using tp_key_r = Result<tbl_row_t, ParserError>;

/**
 * @brief Job buffer — a buffer reference with protocol parsing for job
 * processing.
 *
 * Inherits buf_ref for buffer ownership and adds binary protocol field
 * parsing. Tailored for use within the job class.
 *
 * Input format:
 * | op (1) | op_id (1) | txn_id (4) | ack (1) | table_id (1) | tp_key (1) |
 * data... |
 */
class job_buf : public buf_ref {
 public:
  /** @brief Default constructor — creates empty job buffer */
  constexpr job_buf() noexcept : buf_ref(), buffer_size_(0) {}

  /** @brief Construct from an existing buf_ref, taking ownership */
  job_buf(buf_ref buf, std::size_t size) noexcept;

  /** @brief Get the underlying buffer */
  buf_t& get_buffer() const noexcept {
    return buffer();
  }

  /** @brief Get the operation type */
  Result<OpType, ParserError> get_op() const noexcept;

  /** @brief Get the operation ID */
  Result<op_t, ParserError> get_op_id() const noexcept;

  /** @brief Get the transaction ID */
  Result<txn_id_t, ParserError> get_txn() const noexcept;

  /** @brief Get the table identifier */
  Result<tp_id_t, ParserError> get_tbl() const noexcept;

  /** @brief Get the tuple key */
  Result<tbl_row_t, ParserError> get_key() const noexcept;

  /** @brief Get the acknowledgment */
  Result<OpAck, ParserError> get_ack() const noexcept;

  /** @brief Get a string field from the data section */
  std::string get_string() const noexcept;

  /** @brief Get the checksum */
  cksum_t get_checksum() const noexcept;

  /** @brief Set the acknowledgment field */
  void set_ack(OpAck ack) noexcept;

  /** @brief Set the transaction ID field */
  void set_txn(txn_id_t txn) noexcept;

  /** @brief Set the tuple key field */
  void set_key(tbl_row_t key) noexcept;

  /** @brief Set the buffer size */
  void set_buffer_size(std::size_t size) noexcept;

  /** @brief Get the buffer size */
  std::size_t get_buffer_size() const noexcept;

  /** @brief Get the offset where tuple data begins */
  std::size_t get_tuple_offset() const noexcept;

 private:
  std::size_t buffer_size_;

  template <typename T>
  bool parse_field(T& value, FieldOffset offset) const noexcept;

  template <typename T>
  void set_field(const T& value, FieldOffset offset) noexcept;
};

template <typename T>
bool job_buf::parse_field(T& value, FieldOffset offset) const noexcept {
  const auto off = static_cast<std::size_t>(offset);
  if (off + sizeof(T) > buffer_size_) {
    return false;
  }
  std::memcpy(&value, buffer().data() + off, sizeof(T));
  return true;
}

template <typename T>
void job_buf::set_field(const T& value, FieldOffset offset) noexcept {
  std::memcpy(buffer().data() + static_cast<std::size_t>(offset), &value,
              sizeof(T));
}

}  // namespace PawnDB

#endif  // PAWNDB_TYPES_JOB_BUF_H
