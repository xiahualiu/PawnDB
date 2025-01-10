#include "pawndb/types/student_tuple.h"

#include "pawndb/params.h"

namespace PawnDB {

// Required trait implementations
void StudentTuple::trait_set_checksum() noexcept {
  checksum_ = trait_cal_checksum();
}

cksum_t StudentTuple::trait_val_checksum() const noexcept {
  return checksum_ == trait_cal_checksum();
}

void StudentTuple::trait_set_tickstamp(const tick_t _tickstamp) noexcept {
  this->tickstamp_ = _tickstamp;
}

tick_t StudentTuple::trait_read_tickstamp() const noexcept {
  return tickstamp_;
}

std::size_t StudentTuple::trait_serialize(char* buffer,
                                          std::size_t offset) const noexcept {
  auto buffer_ptr = buffer + offset;
  std::memcpy(buffer_ptr, name_.data(), sizeof(name_));
  buffer_ptr += sizeof(name_);
  std::memcpy(buffer_ptr, &age_, sizeof(age_));
  buffer_ptr += sizeof(age_);
  std::memcpy(buffer_ptr, const_cast<const cksum_t*>(&checksum_),
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
  std::memcpy(const_cast<cksum_t*>(&checksum_), buffer_ptr, sizeof(checksum_));
  if (!trait_val_checksum()) {
    return SerializerError::CheckSumFailed;
  } else {
    return sizeof(name_) + sizeof(age_) + sizeof(checksum_);
  }
}

cksum_t StudentTuple::trait_cal_checksum() const noexcept {
  cksum_t result = 0;
  for (char c : name_) {
    result += static_cast<cksum_t>(c);
  }
  result += static_cast<cksum_t>(age_);
  return result;
}

}  // namespace PawnDB
