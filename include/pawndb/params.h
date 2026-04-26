/**
 * @file params.h
 * @author Xiahua Liu @xiahualiu
 * @brief PawnDB general parameters.
 * @version 0.1
 * @date 2026-04-26
 * @copyright MIT License
 */

#ifndef PAWNDB_PARAMS_H
#define PAWNDB_PARAMS_H

#include <array>
#include <chrono>
#include <cstdint>

#include "pawndb/result.h"

namespace PawnDB {

/// @brief Table row size type, uint8_t is enough if the max row size is fewer
/// than 255.
using tbl_row_t = std::uint8_t;

/// @brief Tuple id type, uint8_t is enough if the max tuple id is fewer than
/// 255.
using tp_id_t = std::uint8_t;

/// @brief Data id type, uint8_t is enough if the max data id is fewer than
/// 255.
using d_id_t = std::uint8_t;

/// @brief Transaction id type, uint32_t is enough if the max transaction id
/// is fewer than 4,294,967,295.
using txn_id_t = std::uint32_t;

/// @brief Operation id type, uint8_t is enough in most cases.
using op_t = std::uint8_t;

/// @brief Lock type, uint8_t is enough if the max shared lock number on an
/// item is fewer than 127.
using lk_t = std::int8_t;

/// @brief Each buffer entry has 32768 bytes.
constexpr std::size_t BUFFER_WIDTH = 32768;

/// @brief Buffer type, each buffer has 32768 bytes.
using buffer_t = std::array<char, BUFFER_WIDTH>;

/// @brief Tick type, uint32_t is enough for most cases.
using tick_t = std::uint32_t;

/// @brief Checksum type, uint32_t is enough for most cases.
using cksum_t = std::uint32_t;

/// @brief Buffer pool has 64 rows. It must be big enough because buffer pool
/// full is UB in PawnDB.
constexpr tbl_row_t BUFFER_ROWS = 64;

/// @brief Each channel has 16 rows.
constexpr tbl_row_t MAX_ITEM_PER_CHANNEL = 16;

/// @brief Buffer alignment. Should be same as the system page size.
constexpr std::size_t BUFFER_ALIGNMENT = 4096;

/// @brief PawnDB is only able to handle this many transactions at the same
/// time.
constexpr tbl_row_t MAX_TRANSACTIONS = 64;

/// @brief The maximum number of locks that can be held by a transaction at
/// the same time.
constexpr tbl_row_t MAX_LOCK_PER_TRANSACTION = 16;

/// @brief The maximum number of commit items a transaction can have at the
/// same time.
constexpr tbl_row_t MAX_COMMIT_PER_TRANSACTION = 16;

/// @brief The max wait time for a operation.
constexpr auto WAIT_TIMEOUT = std::chrono::milliseconds(3000);

/// @brief The max timeout retry times. Any operation will be considered
/// failed if it exceeds this number. Note commit operation will not be retried.
constexpr std::uint8_t MAX_TIMEOUT_RETRY = 8;

/// @brief Socket file path. Must be smaller than sockaddr_un::sun_path.
constexpr char UNIX_SOCKET_PATH[] = "/tmp/pawndb.sock";

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
  ABORT_TXN       /**< Abort transaction */
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

/** @brief Queue operation error codes */
enum class QueueError {
  None,   /**< Operation successful */
  Empty,  /**< Queue is empty */
  Full,   /**< Queue is full */
  Timeout /**< Operation timed out */
};

/** @brief Table operation error codes */
enum class TableError {
  None,     /**< Operation successful */
  Full,     /**< Table is full */
  NotFound, /**< Entry not found */
  Conflict  /**< Entry already exists */
};

/** @brief Lock type enumeration */
enum class LockType : std::uint8_t {
  SHARED,   /**< Shared (read) lock */
  EXCLUSIVE /**< Exclusive (write) lock */
};

/** @brief Lock operation error codes */
enum class LockError {
  None,     /**< Operation successful */
  Full,     /**< Lock table full */
  NotFound, /**< Lock not found */
  Conflict  /**< Lock already held */
};

/** @brief Tuple table operation error codes */
enum class TupleTableError {
  None,    /**< Operation successful */
  Full,    /**< Table at capacity */
  Timeout, /**< Lock wait timeout */
  NotFound /**< Entry not found */
};

/** @brief Buffer operation error codes */
enum class BufferError {
  None, /**< Operation successful */
  Full  /**< Buffer pool full */
};

/** @brief Serializer error codes */
enum class SerializerError {
  None,         /**< No error */
  ExceededWidth /**< Data exceeds buffer width */
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

}  // namespace PawnDB

#endif  // PAWNDB_PARAMS_H
