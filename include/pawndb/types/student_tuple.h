#ifndef PAWNDB_TYPES_STUDENT_TUPLE_H
#define PAWNDB_TYPES_STUDENT_TUPLE_H

#include <array>
#include <cstdint>
#include <cstring>

#include "pawndb/traits/serializer.h"
#include "pawndb/traits/tuple.h"

namespace PawnDB {

class StudentTuple : public TupleTrait<StudentTuple>,
                     public SerializerTrait<StudentTuple> {
 public:
  static constexpr std::size_t NAME_LENGTH = 32;

  StudentTuple() : name_{}, age_(), checksum_(0), tickstamp_(0) {}

  StudentTuple(const StudentTuple& other)
      : name_(other.name_), age_(other.age_), checksum_(other.checksum_) {}

  StudentTuple& operator=(const StudentTuple& other) {
    name_ = other.name_;
    age_ = other.age_;
    checksum_ = other.checksum_;
    return *this;
  }

  // Required trait
  void trait_set_checksum() noexcept;
  cksum_t trait_val_checksum() const noexcept;
  void trait_set_tickstamp(const tick_t _tickstamp) noexcept;
  tick_t trait_read_tickstamp() const noexcept;
  // SerializerTrait
  std::size_t trait_serialize(char* buffer, std::size_t offset) const noexcept;
  SerializeR trait_deserialize(const char* buffer, std::size_t offset) noexcept;

 private:
  cksum_t trait_cal_checksum() const noexcept;

  std::array<char, NAME_LENGTH> name_;
  std::uint8_t age_;

  volatile cksum_t checksum_;
  tick_t tickstamp_;
};

}  // namespace PawnDB

#endif  // PAWNDB_TYPES_STUDENT_TUPLE_H
