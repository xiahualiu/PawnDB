#ifndef PAWNDB_TRAITS_PARSER_H
#define PAWNDB_TRAITS_PARSER_H

#include "pawndb/params.h"
#include "pawndb/result.h"

namespace PawnDB {

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

enum class ParserError {
  None,
  ReadAfterEnd,
  InvalidValue,
};

template <typename Derived>
class ParserTrait {
 public:
  using OpR = Result<OpType, ParserError>;
  OpR get_op() noexcept {
    return static_cast<const Derived*>(this)->trait_get_op();
  }

  using OpIdR = Result<op_t, ParserError>;
  OpIdR get_op_id() noexcept {
    return static_cast<const Derived*>(this)->trait_get_op_id();
  }

  using TxnIdR = Result<txn_id_t, ParserError>;
  TxnIdR get_txn() noexcept {
    return static_cast<const Derived*>(this)->trait_get_txn();
  }

  using TblIdR = Result<tp_id_t, ParserError>;
  TblIdR get_tbl() noexcept {
    return static_cast<const Derived*>(this)->trait_get_tbl();
  }

  constexpr std::size_t get_key_offset() noexcept {
    return static_cast<const Derived*>(this)->trait_get_key_offset();
  }

  void set_ack(OpAck ack) noexcept {
    static_cast<Derived*>(this)->trait_set_ack(ack);
  }

  void set_txn_id(txn_id_t txn) noexcept {
    static_cast<Derived*>(this)->trait_set_txn_id(txn);
  }

  void set_buffer_size(buf_size_t size) noexcept {
    static_cast<Derived*>(this)->trait_set_buffer_size(size);
  }

  std::size_t get_buffer_size() noexcept {
    static_cast<Derived*>(this)->trait_get_buffer_size();
  }

 protected:
  ParserTrait() = default;
  ~ParserTrait() = default;
};

}  // namespace PawnDB

#endif  // PAWNDB_TRAITS_PARSER_H
