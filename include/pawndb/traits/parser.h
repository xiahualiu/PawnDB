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
  SUCCESS = 1, /**< Operation successful */
  DEAD_TXN,    /**< Transaction is dead */
  BAD_OP,      /**< Invalid operation */
  BAD_TXN,     /**< Invalid transaction */
  BAD_TABLE,   /**< Invalid table */
  BAD_TP,      /**< Invalid tuple */
  BAD_ACCESS,  /**< Access violation */
  BAD_PHASE,   /**< Invalid phase */
  BAD_DATA,    /**< Invalid data */
  COMMIT_FULL, /**< Commit table full */
  ABORTED,     /**< Transaction aborted */
  TIMEOUT,     /**< Operation timeout */
  BUSY,        /**< Resource busy */
};

/**
 * @brief Parser error codes
 */
enum class ParserError {
  None,         /**< No error */
  ReadAfterEnd, /**< Read past end of input */
  InvalidValue, /**< Invalid value encountered */
};

/**
 * @brief CRTP base class for operation parsing
 * @tparam Derived The derived parser class
 *
 * Required trait implementations:
 * - trait_get_op() -> OpR
 * - trait_get_op_id() -> OpIdR
 * - trait_get_txn() -> TxnIdR
 * - trait_get_tbl() -> TblIdR
 * - trait_get_key() -> TpKeyR
 * - trait_set_ack(OpAck)
 */
template <typename Derived>
class ParserTrait {
 public:
  /** @brief Operation type result */
  using op_r = Result<OpType, ParserError>;

  /** @brief Operation ID result */
  using op_id_r = Result<op_t, ParserError>;

  /** @brief Transaction ID result */
  using txn_id_r = Result<txn_id_t, ParserError>;

  /** @brief Table ID result */
  using tbl_id_r = Result<tp_id_t, ParserError>;

  /** @brief Table ID result */
  using tp_key_r = Result<tbl_row_t, ParserError>;

  /** @brief Get operation type
   *  @return Result containing operation type or error */
  op_r get_op() const noexcept {
    return static_cast<const Derived*>(this)->trait_get_op();
  }

  /** @brief Get operation ID
   *  @return Result containing operation ID or error */
  op_id_r get_op_id() const noexcept {
    return static_cast<const Derived*>(this)->trait_get_op_id();
  }

  /** @brief Get transaction ID
   *  @return Result containing transaction ID or error */
  txn_id_t get_txn() const noexcept {
    return static_cast<const Derived*>(this)->trait_get_txn();
  }

  /** @brief Get table ID
   *  @return Result containing table ID or error */
  tbl_id_r get_tbl() const noexcept {
    return static_cast<const Derived*>(this)->trait_get_tbl();
  }

  /** @brief Get tuple key
   *  @return Result containing tuple key or error */
  tp_key_r get_key() const noexcept {
    return static_cast<const Derived*>(this)->trait_get_key();
  }

  /** @brief Get tuple offset in the buffer
   *  @return Tuple offset */
  constexpr std::size_t get_tuple_offset() const noexcept {
    return static_cast<const Derived*>(this)->trait_get_tuple_offset();
  }

  /** @brief Set operation acknowledgment status
   *  @param _ack Acknowledgment status */
  void set_ack(OpAck _ack) noexcept {
    static_cast<Derived*>(this)->trait_set_ack(_ack);
  }

  /** @brief Set transaction id
   * @param _txn Transaction id */
  void set_txn_id(txn_id_t _txn) noexcept {
    static_cast<Derived*>(this)->trait_set_txn_id(_txn);
  }

  /** @brief Set buffer size for parsing
   * @param _size Buffer size */
  void set_buffer_size(std::size_t _size) noexcept {
    static_cast<Derived*>(this)->trait_set_buffer_size(_size);
  }

  /** @brief Set key field in the buffer
   * @param _key Key value */
  void set_key(tbl_row_t _key) noexcept {
    static_cast<Derived*>(this)->trait_set_key(_key);
  }

  /** @brief Get buffer size
   * @return Buffer size */
  std::size_t get_buffer_size() const noexcept {
    return static_cast<const Derived*>(this)->trait_get_buffer_size();
  }

  /** @brief Get buffer
   * @return Buffer */
  buffer_t& get_buffer() noexcept {
    return static_cast<Derived*>(this)->trait_get_buffer();
  }

 protected:
  ParserTrait() = default;
  ~ParserTrait() = default;
};

}  // namespace PawnDB

#endif  // PAWNDB_TRAITS_PARSER_H
