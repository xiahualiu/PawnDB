#include "pawndb/types/table_tuple_key.h"

namespace PawnDB {

TableTupleKey::TableTupleKey(tp_id_t table_id, tbl_row_t tuple_key) noexcept
    : table_id_(table_id), tuple_key_(tuple_key) {}

TableTupleKey::TableTupleKey(const TableTupleKey& other) noexcept
    : table_id_(other.table_id_), tuple_key_(other.tuple_key_) {}

TableTupleKey& TableTupleKey::operator=(const TableTupleKey& other) {
  table_id_ = other.table_id_;
  tuple_key_ = other.tuple_key_;
  return *this;
}

void TableTupleKey::trait_assemble(std::uint8_t table_id,
                                   std::uint8_t tuple_key) noexcept {
  table_id_ = table_id;
  tuple_key_ = tuple_key;
}

std::pair<std::uint8_t, std::uint8_t> TableTupleKey::trait_disassemble()
    const noexcept {
  return {table_id_, tuple_key_};
}

// HashTrait implementation
std::size_t TableTupleKey::trait_hash() const noexcept {
  return table_id_;
}

// EqTrait implementation
bool TableTupleKey::trait_equals(const TableTupleKey& other) const noexcept {
  return table_id_ == other.table_id_ && tuple_key_ == other.tuple_key_;
}

}  // namespace PawnDB
