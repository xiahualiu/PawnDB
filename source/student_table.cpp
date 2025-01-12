#include "pawndb/types/student_table.h"

#include <cstddef>
#include <cstring>

#include "pawndb/params.h"
#include "pawndb/traits/hash_table.h"

namespace PawnDB {

StudentTableEntry::StudentTableEntry(const StudentTableEntry& other) noexcept
    : checksum_(other.checksum_),
      tickstamp_(other.tickstamp_),
      name_(other.name_),
      age_(other.age_),
      key_(other.key_),
      lock_(other.lock_),
      is_used_(other.is_used_),
      is_deleted_(other.is_deleted_) {}

StudentTableEntry& StudentTableEntry::operator=(
    const StudentTableEntry& other) noexcept {
  checksum_ = other.checksum_;
  tickstamp_ = other.tickstamp_;
  name_ = other.name_;
  age_ = other.age_;
  key_ = other.key_;
  lock_ = other.lock_;
  is_used_ = other.is_used_;
  is_deleted_ = other.is_deleted_;
  return *this;
}

void StudentTableEntry::trait_set_checksum() noexcept {
  checksum_ = compute_checksum();
}

bool StudentTableEntry::trait_val_checksum() const noexcept {
  return checksum_ == compute_checksum();
}

void StudentTableEntry::trait_set_tickstamp(const tick_t _tickstamp) noexcept {
  tickstamp_ = _tickstamp;
}

tick_t StudentTableEntry::trait_read_tickstamp() const noexcept {
  return tickstamp_;
}

StudentTableEntry::serial_r StudentTableEntry::trait_serialize(
    BufferRef _buffer, std::size_t _offset) const noexcept {
  auto buffer_ptr = _buffer.buffer().data() + _offset;
  std::memcpy(buffer_ptr, name_.data(), NAME_LENGTH);
  buffer_ptr += NAME_LENGTH;
  std::memcpy(buffer_ptr, &age_, sizeof(std::uint8_t));
  buffer_ptr += sizeof(std::uint8_t);
  std::memcpy(buffer_ptr, const_cast<cksum_t*>(&checksum_), sizeof(cksum_t));
  return sizeof(cksum_t) + NAME_LENGTH + sizeof(std::uint8_t);
}

StudentTableEntry::serial_r StudentTableEntry::trait_deserialize(
    BufferRef _buffer, std::size_t _offset) noexcept {
  auto buffer_ptr = _buffer.buffer().data() + _offset;
  std::memcpy(name_.data(), buffer_ptr, NAME_LENGTH);
  buffer_ptr += NAME_LENGTH;
  std::memcpy(&age_, buffer_ptr, sizeof(std::uint8_t));
  buffer_ptr += sizeof(std::uint8_t);
  std::memcpy(const_cast<cksum_t*>(&checksum_), buffer_ptr, sizeof(cksum_t));
  return sizeof(cksum_t) + NAME_LENGTH + sizeof(std::uint8_t);
}

cksum_t StudentTableEntry::compute_checksum() const noexcept {
  cksum_t result = 0;
  for (char c : name_) {
    result += static_cast<cksum_t>(c);
  }
  result += static_cast<cksum_t>(age_);
  return result;
}

StudentTableEntry StudentTableEntry::trait_clone() const noexcept {
  return *this;
}

void StudentTableEntry::trait_copy(const StudentTableEntry& other) noexcept {
  checksum_ = other.checksum_;
  tickstamp_ = other.tickstamp_;
  name_ = other.name_;
  age_ = other.age_;
  key_ = other.key_;
  lock_ = other.lock_;
  is_used_ = other.is_used_;
  is_deleted_ = other.is_deleted_;
}

std::size_t StudentTableEntry::trait_hash() const noexcept {
  return key_;
}

StudentTable::table_r StudentTable::trait_insert(
    const entry_type& _entry) noexcept {
  auto lock = std::unique_lock<std::mutex>(mtx_);
  not_full_.wait(lock, [&]() { return !trait_full(); });
  auto idx = tuple_key_ % Rows;
  auto start = idx;
  do {
    if (!table_[idx].is_used_ || table_[idx].is_deleted_) {
      table_[idx] = _entry;
      table_[idx].key_ = tuple_key_;
      table_[idx].lock_ = 0;
      table_[idx].is_used_ = true;
      table_[idx].is_deleted_ = false;
      tuple_key_++;
      size_++;
      return table_[idx];
    }
    idx = (idx + 1) % Rows;
  } while (idx != start);
  return TableError::Full;
}

StudentTable::table_r StudentTable::trait_search(
    const key_type& _key) noexcept {
  auto lock = std::unique_lock<std::mutex>(mtx_);
  auto idx = _key % Rows;
  auto start = idx;
  do {
    if (!table_[idx].is_used_) {
      return TableError::NotFound;
    }
    if (table_[idx].key_ == _key && !table_[idx].is_deleted_) {
      return table_[idx];
    }
    idx = (idx + 1) % Rows;
  } while (idx != start);
  return TableError::NotFound;
}

TableError StudentTable::trait_remove(const key_type& _key) noexcept {
  auto lock = std::unique_lock<std::mutex>(mtx_);
  auto idx = _key % Rows;
  auto start = idx;
  do {
    if (!table_[idx].is_used_) {
      return TableError::NotFound;
    }
    if (table_[idx].key_ == _key && !table_[idx].is_deleted_) {
      table_[idx].is_deleted_ = true;
      size_--;
      return TableError::None;
    }
    idx = (idx + 1) % Rows;
  } while (idx != start);
  return TableError::NotFound;
}

StudentTable::table_r StudentTable::trait_write(
    const entry_type& _entry) noexcept {
  auto lock = std::unique_lock<std::mutex>(mtx_);
  auto idx = _entry.key_ % Rows;
  auto start = idx;
  do {
    if (!table_[idx].is_used_) {
      return TableError::NotFound;
    }
    if (table_[idx].key_ == _entry.key_ && !table_[idx].is_deleted_) {
      table_[idx].name_ = _entry.name_;
      table_[idx].age_ = _entry.age_;
      table_[idx].trait_set_checksum();
      return table_[idx];
    }
    idx = (idx + 1) % Rows;
  } while (idx != start);
  return TableError::NotFound;
}

StudentTable::ttable_r StudentTable::trait_wait_shared() noexcept {
  auto lock = std::unique_lock<std::mutex>(mtx_);
  ttable_r result = TupleTableError::Timeout;
  if (not_empty_.wait_for(lock, WAIT_TIMEOUT,
                          [&]() { return !trait_empty(); })) {
    if (s_available_.wait_for(lock, WAIT_TIMEOUT, [&]() {
          for (tbl_row_t i = 0; i < Rows; i++) {
            if (table_[i].is_used_ && !table_[i].is_deleted_ &&
                table_[i].lock_ >= 0) {
              table_[i].lock_++;
              result = table_[i];
              return true;
            }
          }
          return false;
        })) {
      return result;
    }
  }
  return result;
}

StudentTable::ttable_r StudentTable::trait_wait_exclusive() noexcept {
  auto lock = std::unique_lock<std::mutex>(mtx_);
  ttable_r result = TupleTableError::Timeout;

  if (not_empty_.wait_for(lock, WAIT_TIMEOUT,
                          [&]() { return !trait_empty(); })) {
    if (x_available_.wait_for(lock, WAIT_TIMEOUT, [&]() {
          for (tbl_row_t i = 0; i < Rows; i++) {
            if (table_[i].is_used_ && !table_[i].is_deleted_ &&
                table_[i].lock_ == 0) {
              table_[i].lock_ = -1;
              result = table_[i];
              return true;
            }
          }
          return false;
        })) {
      return result;
    }
  }
  return result;
}

TupleTableError StudentTable::trait_promote(const key_type& _key) noexcept {
  auto lock = std::unique_lock<std::mutex>(mtx_);
  auto idx = _key % Rows;
  auto start = idx;
  do {
    if (table_[idx].key_ == _key && !table_[idx].is_deleted_) {
      if (x_available_.wait_for(lock, WAIT_TIMEOUT,
                                [&]() { return table_[idx].lock_ == 1; })) {
        table_[idx].lock_ = -1;
        return TupleTableError::None;
      }
      return TupleTableError::Timeout;
    }
    idx = (idx + 1) % Rows;
  } while (idx != start);
  return TupleTableError::NotFound;
}

void StudentTable::trait_release(const key_type& _key) noexcept {
  auto lock = std::unique_lock<std::mutex>(mtx_);
  auto idx = _key % Rows;
  auto start = idx;

  do {
    if (table_[idx].key_ == _key && !table_[idx].is_deleted_) {
      if (table_[idx].lock_ == -1) {
        table_[idx].lock_ = 0;
      } else {
        table_[idx].lock_--;
      }
      return;
    }
    idx = (idx + 1) % Rows;
  } while (idx != start);
}

void StudentTable::trait_notify_s() noexcept {
  s_available_.notify_all();
}

void StudentTable::trait_notify_x() noexcept {
  x_available_.notify_one();
}

void StudentTable::trait_notify_not_empty() noexcept {
  not_empty_.notify_all();
}

void StudentTable::trait_notify_not_full() noexcept {
  not_full_.notify_one();
}

std::size_t StudentTable::trait_size() const noexcept {
  return size_;
}

bool StudentTable::trait_empty() const noexcept {
  return size_ == 0;
}

bool StudentTable::trait_full() const noexcept {
  return size_ == Rows;
}

}  // namespace PawnDB
