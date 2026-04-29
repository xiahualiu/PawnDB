#include "pawndb/types/lock.h"

namespace PawnDB {

lock_entry::lock_entry(const unique_key& _key, LockType _type) noexcept
    : key_(_key), type_(_type), is_used_(true), is_deleted_(false) {}

lock_entry::lock_entry(const lock_entry& other) noexcept {
  key_ = other.key_;
  type_ = other.type_;
}

lock_entry& lock_entry::operator=(const lock_entry& other) noexcept {
  key_ = other.key_;
  type_ = other.type_;
  return *this;
}

lock_entry lock_entry::copy() const noexcept {
  return lock_entry(*this);
}

void lock_entry::copy_from(const lock_entry& other) noexcept {
  key_ = other.key_;
  type_ = other.type_;
}

LockType lock_entry::lock_type() const noexcept {
  return type_;
}

const unique_key& lock_entry::key() const noexcept {
  return key_;
}

}  // namespace PawnDB
