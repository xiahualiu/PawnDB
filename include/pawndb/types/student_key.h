#ifndef PAWNDB_TYPES_U8_KEY_H
#define PAWNDB_TYPES_U8_KEY_H

#include <cstdint>
#include <cstring>

#include "pawndb/traits/serializer.h"
#include "pawndb/traits/sized.h"
#include "pawndb/traits/tuple_key.h"

namespace PawnDB {

class StudentKey : public TupleKeyTrait<StudentKey>,
                   public Sized<StudentKey>,
                   public SerializerTrait<StudentKey> {
 public:
  StudentKey() : value_(0) {}

  StudentKey(const StudentKey& other) : value_(other.value_) {}

  StudentKey& operator=(const StudentKey& other) {
    value_ = other.value_;
    return *this;
  }

  std::size_t trait_next() const noexcept { return value_ + 1; }

  std::size_t trait_hash() const noexcept { return value_; }

  bool trait_equals(const std::size_t& other) const noexcept {
    return value_ == other;
  }

  std::size_t trait_size() const noexcept { return sizeof(value_); }

  bool trait_equals(const StudentKey& _other) const noexcept {
    return value_ == _other.value_;
  }

  std::size_t trait_serialize(char* _buffer,
                              std::size_t _offset) const noexcept {
    std::memcpy(_buffer + _offset, &value_, sizeof(value_));
    return sizeof(value_);
  }

  SerializeR trait_deserialize(const char* _buffer,
                               std::size_t _offset) noexcept {
    std::memcpy(&value_, _buffer + _offset, sizeof(value_));
    return sizeof(value_);
  }

 private:
  std::uint8_t value_;
};

}  // namespace PawnDB

#endif  // PAWNDB_TYPES_SIMPLE_KEY_H
