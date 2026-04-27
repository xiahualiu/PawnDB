#include "pawndb/types/unique_key.h"

#include <utility>

namespace PawnDB {

// Value constructor
unique_key::unique_key(tp_id_t table_id, tbl_row_t tuple_key) noexcept
    : table_id_(table_id), tuple_key_(tuple_key) {}

// Copy constructor
unique_key::unique_key(const unique_key& other) noexcept
    : table_id_(other.table_id_), tuple_key_(other.tuple_key_) {}

// Copy assignment
unique_key& unique_key::operator=(const unique_key& other) noexcept {
  table_id_ = other.table_id_;
  tuple_key_ = other.tuple_key_;
  return *this;
}

void unique_key::assemble_(std::uint8_t table_id,
                           std::uint8_t tuple_key) noexcept {
  table_id_ = table_id;
  tuple_key_ = tuple_key;
}

std::pair<std::uint8_t, std::uint8_t> unique_key::disassemble_()
    const noexcept {
  return {table_id_, tuple_key_};
}

std::size_t unique_key::hash_() const noexcept {
  return table_id_;
}

bool unique_key::equals_(const unique_key& other) const noexcept {
  return table_id_ == other.table_id_ && tuple_key_ == other.tuple_key_;
}

unique_key unique_key::copy_() const noexcept {
  return unique_key(*this);
}

void unique_key::copy_from_(const unique_key& other) noexcept {
  tuple_key_ = other.tuple_key_;
}

unique_key unique_key::move_() noexcept {
  return std::move(*this);
}

void unique_key::move_from_(unique_key&& other) noexcept {
  *this = std::move(other);
}

}  // namespace PawnDB
