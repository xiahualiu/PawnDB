#include "pawndb/types/table_tuple_key.h"

#include <utility>

namespace PawnDB {

// Value constructor
TableTupleKey::TableTupleKey(tp_id_t table_id, tbl_row_t tuple_key) noexcept
    : table_id_(table_id), tuple_key_(tuple_key) {}

// Copy constructor
TableTupleKey::TableTupleKey(const TableTupleKey& other) noexcept
    : table_id_(other.table_id_), tuple_key_(other.tuple_key_) {}

// Assignment operator
TableTupleKey& TableTupleKey::operator=(const TableTupleKey& other) {
  table_id_ = other.table_id_;
  tuple_key_ = other.tuple_key_;
  return *this;
}

// CompositeKeyTrait implementation
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

// CopyTrait implementation
TableTupleKey TableTupleKey::trait_copy() const noexcept {
  return TableTupleKey(*this);
}

void TableTupleKey::trait_copy_from(const TableTupleKey& other) noexcept {
  tuple_key_ = other.tuple_key_;
}

// MoveTrait implementation
TableTupleKey TableTupleKey::trait_move() noexcept {
  return std::move(*this);
}

void TableTupleKey::trait_move_from(TableTupleKey&& other) noexcept {
  *this = std::move(other);
}

// EqTrait implementation
bool TableTupleKey::trait_equals(const TableTupleKey& other) const noexcept {
  return table_id_ == other.table_id_ && tuple_key_ == other.tuple_key_;
}

}  // namespace PawnDB
