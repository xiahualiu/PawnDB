#include "pawndb/types/student_tuple.h"

namespace PawnDB {

// Required trait implementations
void StudentTuple::trait_set_checksum() noexcept {
  checksum_ = trait_cal_checksum();
}

StudentTuple::checksum_type StudentTuple::trait_val_checksum() const noexcept {
  return checksum_ == trait_cal_checksum();
}

void StudentTuple::trait_set_tickstamp(
    const tickstamp_type _tickstamp) noexcept {
  this->tickstamp_ = _tickstamp;
}

StudentTuple::tickstamp_type StudentTuple::trait_read_tickstamp()
    const noexcept {
  return tickstamp_;
}

std::size_t StudentTuple::trait_serialize(char* buffer,
                                          std::size_t offset) const noexcept {
  auto buffer_ptr = buffer + offset;
  std::memcpy(buffer_ptr, name_.data(), sizeof(name_));
  buffer_ptr += sizeof(name_);
  std::memcpy(buffer_ptr, &age_, sizeof(age_));
  buffer_ptr += sizeof(age_);
  std::memcpy(buffer_ptr, const_cast<const checksum_type*>(&checksum_),
              sizeof(checksum_));
  return sizeof(name_) + sizeof(age_) + sizeof(checksum_);
}

StudentTuple::SerializeR StudentTuple::trait_deserialize(
    const char* buffer, std::size_t offset) noexcept {
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

StudentTuple::checksum_type StudentTuple::trait_cal_checksum() const noexcept {
  checksum_type result = 0;
  for (char c : name_) {
    result += static_cast<checksum_type>(c);
  }
  result += static_cast<checksum_type>(age_);
  return result;
}

}  // namespace PawnDB
