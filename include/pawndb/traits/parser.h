#ifndef PAWNDB_TRAITS_PARSER_H
#define PAWNDB_TRAITS_PARSER_H

#include "pawndb/params.h"
#include "pawndb/result.h"

namespace PawnDB {

/**
 * @brief Operation types
 */
enum class OpType : std::uint8_t {
  START_TXN = 1,
  COMMIT_TXN,
  ABORT_TXN,
  ADD_TUPLE,
  SHARED_READ,
  EXCLUSIVE_READ,
  YIELD_READ,
  PROMOTE,
  UPDATE,
  DELETE,
  MAX_OP_VALUE,
};

/**
 * @brief Operation acknowledgment status codes
 */
enum class OpAck : std::uint8_t {
  SUCCESS = 1,
  DEAD_TXN,
  BAD_OP,
  BAD_TXN,
  BAD_TABLE,
  BAD_TP,
  BAD_ACCESS,
  BAD_PHASE,
  BAD_DATA,
  COMMIT_FULL,
  ABORTED,
  TIMEOUT,
  BUSY,
};

/**
 * @brief Parser error codes
 */
enum class ParserError {
  None,
  ReadAfterEnd,
  InvalidValue,
};

/**
 * @brief CRTP base class for operation parsing
 * @tparam Derived The derived parser class */
template <typename Derived>
class ParserTrait {
 public:
  using OpR = Result<OpType, ParserError>;

  /** @brief Get operation type
   *  @return Result containing operation type or error */
  OpR get_op() const noexcept {
    return static_cast<const Derived*>(this)->trait_get_op();
  }

  using OpIdR = Result<op_t, ParserError>;

  /** @brief Get operation ID
   *  @return Result containing operation ID or error */
  OpIdR get_op_id() const noexcept {
    return static_cast<const Derived*>(this)->trait_get_op_id();
  }

  using TxnIdR = Result<txn_id_t, ParserError>;

  /** @brief Get transaction ID
   *  @return Result containing transaction ID or error */
  TxnIdR get_txn() const noexcept {
    return static_cast<const Derived*>(this)->trait_get_txn();
  }

  using TblIdR = Result<tp_id_t, ParserError>;

  /** @brief Get table ID
   *  @return Result containing table ID or error */
  TblIdR get_tbl() const noexcept {
    return static_cast<const Derived*>(this)->trait_get_tbl();
  }

  using TpKeyR = Result<tbl_row_t, ParserError>;

  /** @brief Get tuple key
   *  @return Result containing tuple key or error */
  TblIdR get_key() const noexcept {
    return static_cast<const Derived*>(this)->trait_get_key();
  }

  /** @brief Get tuple offset in the buffer
   *  @return Tuple offset */
  constexpr std::size_t get_tuple_offset() const noexcept {
    return static_cast<const Derived*>(this)->trait_get_tuple_offset();
  }

  /** @brief Set operation acknowledgment status
   *  @param ack Acknowledgment status */
  void set_ack(OpAck ack) noexcept {
    static_cast<Derived*>(this)->trait_set_ack(ack);
  }

  /** @brief Set transaction id
   * @param txn Transaction id */
  void set_txn_id(txn_id_t txn) noexcept {
    static_cast<Derived*>(this)->trait_set_txn_id(txn);
  }

  /** @brief Set buffer size for parsing
   * @param size Buffer size */
  void set_buffer_size(std::size_t size) noexcept {
    static_cast<Derived*>(this)->trait_set_buffer_size(size);
  }

  /** @brief Set key field in the buffer
   * @param key Key value */
  void set_key(tbl_row_t key) noexcept {
    static_cast<Derived*>(this)->trait_set_key(key);
  }

  /** @brief Get buffer size
   * @return Buffer size */
  std::size_t get_buffer_size() const noexcept {
    return static_cast<const Derived*>(this)->trait_get_buffer_size();
  }

  /** @brief Get buffer
   * @return Buffer */
  char* get_buffer() noexcept {
    return static_cast<Derived*>(this)->trait_get_buffer();
  }

 protected:
  ParserTrait() = default;
  ~ParserTrait() = default;
};

}  // namespace PawnDB

#endif  // PAWNDB_TRAITS_PARSER_H
