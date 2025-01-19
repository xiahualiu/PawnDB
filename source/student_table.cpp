#include "pawndb/types/student_table.h"

#include <cstddef>
#include <cstring>
#include <string>

#include "pawndb/params.h"
#include "pawndb/result.h"
#include "pawndb/tools.h"
#include "pawndb/traits/table.h"
#include "pawndb/traits/tuple_table.h"

namespace PawnDB {

StudentTuple::StudentTuple(const StudentTuple& other) noexcept
    : checksum_(other.checksum_),
      tickstamp_(other.tickstamp_),
      name_(other.name_),
      age_(other.age_),
      key_(other.key_) {}

StudentTuple::StudentTuple(std::string _name, const std::uint8_t _age,
                           const tbl_row_t _key) noexcept
    : checksum_(0), tickstamp_(0), name_({0}), age_(_age), key_(_key) {
  std::memcpy(name_.data(), _name.data(), _name.size());
  set_checksum();
}

StudentTuple& StudentTuple::operator=(const StudentTuple& other) noexcept {
  checksum_ = other.checksum_;
  tickstamp_ = other.tickstamp_;
  name_ = other.name_;
  age_ = other.age_;
  key_ = other.key_;
  return *this;
}

void StudentTuple::trait_set_checksum() noexcept {
  checksum_ = compute_checksum();
}

bool StudentTuple::trait_val_checksum() const noexcept {
  return checksum_ == compute_checksum();
}

void StudentTuple::trait_set_tickstamp(const tick_t _tickstamp) noexcept {
  tickstamp_ = _tickstamp;
}

tick_t StudentTuple::trait_read_tickstamp() const noexcept {
  return tickstamp_;
}

tbl_row_t StudentTuple::trait_key() const noexcept {
  return key_;
}

StudentTuple::serial_r StudentTuple::trait_serialize(
    BufferRef _buffer, std::size_t _offset) const noexcept {
  auto buffer_ptr = _buffer.buffer().data() + _offset;
  std::memcpy(buffer_ptr, name_.data(), NAME_LENGTH);
  buffer_ptr += NAME_LENGTH;
  std::memcpy(buffer_ptr, &age_, sizeof(std::uint8_t));
  buffer_ptr += sizeof(std::uint8_t);
  std::memcpy(buffer_ptr, const_cast<cksum_t*>(&checksum_), sizeof(cksum_t));
  return sizeof(cksum_t) + NAME_LENGTH + sizeof(std::uint8_t);
}

StudentTuple::serial_r StudentTuple::trait_deserialize(
    BufferRef _buffer, std::size_t _offset) noexcept {
  auto buffer_ptr = _buffer.buffer().data() + _offset;
  std::memcpy(name_.data(), buffer_ptr, NAME_LENGTH);
  buffer_ptr += NAME_LENGTH;
  std::memcpy(&age_, buffer_ptr, sizeof(std::uint8_t));
  buffer_ptr += sizeof(std::uint8_t);
  std::memcpy(const_cast<cksum_t*>(&checksum_), buffer_ptr, sizeof(cksum_t));
  return sizeof(cksum_t) + NAME_LENGTH + sizeof(std::uint8_t);
}

cksum_t StudentTuple::compute_checksum() const noexcept {
  cksum_t result = 0;
  for (char c : name_) {
    result += static_cast<cksum_t>(c);
  }
  result += static_cast<cksum_t>(age_);
  return result;
}

StudentTuple StudentTuple::trait_clone() const noexcept {
  return *this;
}

void StudentTuple::trait_copy(const StudentTuple& other) noexcept {
  checksum_ = other.checksum_;
  tickstamp_ = other.tickstamp_;
  name_ = other.name_;
  age_ = other.age_;
  key_ = other.key_;
}

std::size_t StudentTuple::trait_hash() const noexcept {
  return key_;
}

bool StudentTuple::trait_equals(const StudentTuple& other) const noexcept {
  return key_ == other.key_ && array_cmp(name_, other.name_) == 0 &&
         age_ == other.age_;
}

StudentTable::StudentTable() noexcept
    : table_{}, s_avail_cnt_(0), x_avail_cnt_(0), size_(0), next_key_(0) {}

StudentTable::table_r StudentTable::trait_insert(
    const tuple_t& _tuple) noexcept {
  auto lock = std::unique_lock<std::mutex>(mtx_);
  not_full_.wait(lock, [&]() { return !trait_full(); });
  auto idx = next_key_ % Rows;
  while (true) {
    if (!table_[idx].is_used_ || table_[idx].is_deleted_) {
      table_[idx].tuple_ = _tuple;
      table_[idx].tuple_.key_ = next_key_;
      table_[idx].lock_ = 0;
      table_[idx].is_used_ = true;
      table_[idx].is_deleted_ = false;
      s_avail_cnt_++;
      x_avail_cnt_++;
      next_key_++;
      size_++;
      return table_[idx].tuple_;
    }
    idx = (idx + 1) % Rows;
  }
}

StudentTable::table_r StudentTable::trait_search(const key_t& _key) noexcept {
  auto lock = std::unique_lock<std::mutex>(mtx_);
  auto idx = _key % Rows;
  auto start = idx;
  do {
    if (!table_[idx].is_used_) {
      return TableError::NotFound;
    }
    if (table_[idx].tuple_.key_ == _key && !table_[idx].is_deleted_) {
      return table_[idx].tuple_;
    }
    idx = (idx + 1) % Rows;
  } while (idx != start);
  return TableError::NotFound;
}

Result<StudentTable::Entry&, TableError> StudentTable::_test_get_entry(
    const key_t& _key) noexcept {
  auto lock = std::unique_lock<std::mutex>(mtx_);
  auto idx = _key % Rows;
  auto start = idx;
  do {
    if (!table_[idx].is_used_) {
      return TableError::NotFound;
    }
    if (table_[idx].tuple_.key_ == _key && !table_[idx].is_deleted_) {
      return table_[idx];
    }
    idx = (idx + 1) % Rows;
  } while (idx != start);
  return TableError::NotFound;
}

TableError StudentTable::trait_remove(const key_t& _key) noexcept {
  auto lock = std::unique_lock<std::mutex>(mtx_);
  auto idx = _key % Rows;
  while (true) {
    if (table_[idx].tuple_.key_ == _key && !table_[idx].is_deleted_) {
      table_[idx].is_deleted_ = true;
      size_--;
      return TableError::None;
    }
    idx = (idx + 1) % Rows;
  }
}

TableError StudentTable::trait_write(const tuple_t& _tuple) noexcept {
  auto lock = std::unique_lock<std::mutex>(mtx_);
  auto idx = _tuple.key_ % Rows;
  while (true) {
    if (table_[idx].tuple_.key_ == _tuple.key_ && !table_[idx].is_deleted_) {
      table_[idx].tuple_ = _tuple;
      return TableError::None;
    }
    idx = (idx + 1) % Rows;
  }
}

void StudentTable::trait_clear() noexcept {
  auto lock = std::unique_lock<std::mutex>(mtx_);
  for (tbl_row_t i = 0; i < Rows; i++) {
    table_[i].lock_ = 0;
    table_[i].is_used_ = false;
    table_[i].is_deleted_ = false;
  }
  s_avail_cnt_ = 0;
  x_avail_cnt_ = 0;
  size_ = 0;
  next_key_ = 0;
}

StudentTable::ttable_r StudentTable::trait_wait_shared() noexcept {
  auto lock = std::unique_lock<std::mutex>(mtx_);
  if (not_empty_.wait_for(lock, WAIT_TIMEOUT,
                          [&]() { return !trait_empty(); })) {
    if (s_available_.wait_for(lock, WAIT_TIMEOUT,
                              [&]() { return s_avail_cnt_ > 0; })) {
      for (tbl_row_t i = 0; i < Rows; i++) {
        if (table_[i].is_used_ && !table_[i].is_deleted_ &&
            table_[i].lock_ >= 0) {
          table_[i].lock_++;
          if (table_[i].lock_ == 1) {
            x_avail_cnt_--;
          }
          return table_[i].tuple_;
        }
      }
      return TupleTableError::NotFound;
    } else {
      return TupleTableError::Timeout;
    }
  }
  return TupleTableError::Timeout;
}

StudentTable::ttable_r StudentTable::trait_wait_exclusive() noexcept {
  auto lock = std::unique_lock<std::mutex>(mtx_);
  if (not_empty_.wait_for(lock, WAIT_TIMEOUT,
                          [&]() { return !trait_empty(); })) {
    if (x_available_.wait_for(lock, WAIT_TIMEOUT,
                              [&]() { return x_avail_cnt_ > 0; })) {
      for (tbl_row_t i = 0; i < Rows; i++) {
        if (table_[i].is_used_ && !table_[i].is_deleted_ &&
            table_[i].lock_ == 0) {
          table_[i].lock_ = -1;
          x_avail_cnt_--;
          s_avail_cnt_--;
          return table_[i].tuple_;
        }
      }
      return TupleTableError::NotFound;
    } else {
      return TupleTableError::Timeout;
    }
  } else {
    return TupleTableError::Timeout;
  }
}

TupleTableError StudentTable::trait_promote(const key_t& _key) noexcept {
  auto lock = std::unique_lock<std::mutex>(mtx_);
  auto idx = _key % Rows;
  while (true) {
    if (table_[idx].tuple_.key_ == _key && !table_[idx].is_deleted_) {
      if (x_available_.wait_for(lock, WAIT_TIMEOUT,
                                [&]() { return table_[idx].lock_ == 1; })) {
        s_avail_cnt_--;
        table_[idx].lock_ = -1;
        return TupleTableError::None;
      }
      return TupleTableError::Timeout;
    }
    idx = (idx + 1) % Rows;
  }
}

void StudentTable::trait_release(const key_t& _key) noexcept {
  auto lock = std::unique_lock<std::mutex>(mtx_);
  auto idx = _key % Rows;
  while (true) {
    if (table_[idx].tuple_.key_ == _key && !table_[idx].is_deleted_) {
      if (table_[idx].lock_ == -1) {
        table_[idx].lock_ = 0;
        x_avail_cnt_++;
        s_avail_cnt_++;
      } else {
        table_[idx].lock_--;
        if (table_[idx].lock_ == 0) {
          x_avail_cnt_++;
        }
      }
      return;
    }
    idx = (idx + 1) % Rows;
  }
}

void StudentTable::trait_notify_shared() noexcept {
  if (s_avail_cnt_ > 0) {
    s_available_.notify_all();
  }
}

void StudentTable::trait_notify_exclusive() noexcept {
  if (x_avail_cnt_ > 0) {
    x_available_.notify_all();
  }
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

tbl_row_t StudentTable::_test_s_avail_cnt() const noexcept {
  return s_avail_cnt_;
}

tbl_row_t StudentTable::_test_x_avail_cnt() const noexcept {
  return x_avail_cnt_;
}

}  // namespace PawnDB
