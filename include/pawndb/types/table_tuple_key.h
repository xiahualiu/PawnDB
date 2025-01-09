#ifndef PAWNDB_TYPES_U8_KEY_H
#define PAWNDB_TYPES_U8_KEY_H

#include <cstdint>
#include <cstring>

#include "pawndb/params.h"
#include "pawndb/traits/composite_key.h"

namespace PawnDB {

class TableTupleKey : public CompositeKeyTrait<TableTupleKey> {
 public:
  TableTupleKey() noexcept : table_id_(0), tuple_key_(0) {}

  TableTupleKey(tp_id_t _table_id, tbl_row_t _tuple_key) noexcept
      : table_id_(_table_id), tuple_key_(_tuple_key) {}

  TableTupleKey(const TableTupleKey& _other) noexcept
      : table_id_(_other.table_id_), tuple_key_(_other.tuple_key_) {}

  TableTupleKey& operator=(const TableTupleKey& _other) {
    table_id_ = _other.table_id_;
    tuple_key_ = _other.tuple_key_;
    return *this;
  }

  void trait_assemble(std::uint8_t _table_id,
                      std::uint8_t _tuple_key) noexcept {
    table_id_ = _table_id;
    tuple_key_ = _tuple_key;
  }

  std::pair<std::uint8_t, std::uint8_t> trait_disassemble() const noexcept {
    return {table_id_, tuple_key_};
  }

  std::size_t trait_hash() const noexcept { return table_id_; }

  bool trait_equals(const TableTupleKey& _other) const noexcept {
    return table_id_ == _other.table_id_ && tuple_key_ == _other.tuple_key_;
  }

 private:
  std::uint8_t table_id_;
  std::uint8_t tuple_key_;
};

}  // namespace PawnDB

#endif  // PAWNDB_TYPES_SIMPLE_KEY_H
