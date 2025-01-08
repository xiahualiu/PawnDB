#ifndef PAWNDB_TYPES_U8_KEY_H
#define PAWNDB_TYPES_U8_KEY_H

#include <cstdint>
#include <cstring>

#include "pawndb/traits/composite_key.h"

namespace PawnDB {

class LockRecordKey : public CompositeKeyTrait<LockRecordKey> {
 public:
  LockRecordKey() noexcept : table_id_(0), tuple_key_(0) {}

  LockRecordKey(const LockRecordKey& _other) noexcept
      : table_id_(_other.table_id_), tuple_key_(_other.tuple_key_) {}

  LockRecordKey& operator=(const LockRecordKey& _other) {
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

  std::size_t trait_next() const noexcept { return 0; }

  std::size_t trait_hash() const noexcept { return table_id_; }

  bool trait_equals(const LockRecordKey& _other) const noexcept {
    return table_id_ == _other.table_id_ && tuple_key_ == _other.tuple_key_;
  }

 private:
  std::uint8_t table_id_;
  std::uint8_t tuple_key_;
};

}  // namespace PawnDB

#endif  // PAWNDB_TYPES_SIMPLE_KEY_H
