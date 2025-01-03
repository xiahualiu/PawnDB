/**
 * @file params.h
 * @author Xiahua Liu @xiahualiu
 * @brief PawnDB general parameters.
 * @version 0.1
 * @date 2025-01-02
 *
 * @copyright MIT License
 *
 */

#ifndef PAWNDB_PARAMS_H
#define PAWNDB_PARAMS_H

#include <chrono>
#include <cstdint>

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

/// @brief Operation type, uint8_t is enough if the max operation type is
/// fewer than 255.
using op_t = std::uint8_t;

/// @brief Lock type, uint8_t is enough if the max shared lock number on an
/// item is fewer than 127.
using lk_t = std::int8_t;

/// @brief Buffer size type, uint16_t is enough if the max buffer size is
/// fewer than 65,535. For most systems, the biggest UDP packet size is 65535
/// on the lo interface.
using buf_size_t = std::uint16_t;

/// @brief Each buffer entry has 32768 bytes.
constexpr buf_size_t BUFFER_WIDTH = 32768;

/// @brief Buffer pool has 64 rows. It must be big enough because buffer pool
/// full is UB in PawnDB.
constexpr tbl_row_t BUFFER_ROWS = 64;

/// @brief Each channel has 16 rows.
constexpr tbl_row_t CHANNEL_ROWS = 16;

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
/// failed if it exceeds this number.
constexpr std::uint8_t MAX_TIMEOUT_RETRY = 8;

/// @brief Socket file path. Must be smaller than sockaddr_un::sun_path.
constexpr char UNIX_SOCKET_PATH[] = "/tmp/pawndb.sock";

}  // namespace PawnDB

#endif  // PAWNDB_PARAMS_H
