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

  // Required trait implementations
  void trait_set_checksum() noexcept { checksum_ = trait_cal_checksum(); }

  checksum_type trait_val_checksum() const noexcept {
    return checksum_ == trait_cal_checksum();
  }

  void trait_set_tickstamp(const tickstamp_type _tickstamp) noexcept {
    this->tickstamp_ = _tickstamp;
  }

  tickstamp_type trait_read_tickstamp() const noexcept { return tickstamp_; }

  std::size_t trait_serialize(char* buffer, std::size_t offset) const noexcept {
    auto buffer_ptr = buffer + offset;
    std::memcpy(buffer_ptr, name_.data(), sizeof(name_));
    buffer_ptr += sizeof(name_);
    std::memcpy(buffer_ptr, &age_, sizeof(age_));
    buffer_ptr += sizeof(age_);
    std::memcpy(buffer_ptr, const_cast<const checksum_type*>(&checksum_),
                sizeof(checksum_));
    return sizeof(name_) + sizeof(age_) + sizeof(checksum_);
  }

  SerializeR trait_deserialize(const char* buffer,
                               std::size_t offset) noexcept {
    auto buffer_ptr = buffer + offset;
    std::memcpy(name_.data(), buffer_ptr, sizeof(name_));
    buffer_ptr += sizeof(name_);
    std::memcpy(&age_, buffer_ptr, sizeof(age_));
    buffer_ptr += sizeof(age_);
    std::memcpy(const_cast<checksum_type*>(&checksum_), buffer_ptr,
                sizeof(checksum_));
    if (!trait_val_checksum()) {
      return SerializerError::CheckSumFailed;
    } else {
      return sizeof(name_) + sizeof(age_) + sizeof(checksum_);
    }
  }

 private:
  checksum_type trait_cal_checksum() const noexcept {
    checksum_type result = 0;
    for (char c : name_) {
      result += static_cast<checksum_type>(c);
    }
    result += static_cast<checksum_type>(age_);
    return result;
  }

  std::array<char, NAME_LENGTH> name_;
  std::uint8_t age_;

  volatile checksum_type checksum_;
  tickstamp_type tickstamp_;
};

}  // namespace PawnDB

#endif  // PAWNDB_TYPES_STUDENT_TUPLE_H
