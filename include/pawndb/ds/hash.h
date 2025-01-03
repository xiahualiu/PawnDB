/**
 * @file hash.h
 * @author Xiahua Liu @xiahualiu
 * @brief PawnDB hash table data structure.
 * @version 0.1
 * @date 2025-01-02
 *
 * @copyright MIT License
 *
 */

#ifndef PAWNDB_HASH_H
#define PAWNDB_HASH_H

#include <array>

#include "pawndb/params.h"
#include "pawndb/result.h"

namespace PawnDB {

enum class HashError {
  None,
  NotFound,
  Full,
  Duplicate,
};

template <class KeyType, tbl_row_t Rows>
class Hash {
  static_assert(std::is_integral_v<KeyType>,
                "KeyType must be an integral type!");
  static_assert(Rows > 0, "Rows must be greater than 0!");

 public:
  struct Entry {
    KeyType key;
    bool is_used;
    bool is_deleted;
  };

  constexpr Hash() noexcept : table(), size() {}

  inline bool empty() const noexcept { return size == 0; }
  inline bool full() const noexcept { return size == Rows; }

  using IndexR = Result<tbl_row_t, HashError>;

  IndexR search(KeyType _key) const noexcept {
    auto oi = hash(_key);
    auto i = oi;
    do {
      if (!table[i].is_used && !table[i].is_deleted) {
        return HashError::NotFound;
      } else if (table[i].is_used && table[i].key == _key) {
        return i;
      }
      i = (i + 1) % Rows;
    } while (i != oi);
    return HashError::NotFound;
  }

  IndexR insert(KeyType _key) noexcept {
    auto oi = hash(_key);
    auto i = oi;
    do {
      if (!table[i].is_used || table[i].is_deleted) {
        table[i] = {_key, true, false};
        size++;
        return i;
      } else if (table[i].key == _key) {
        return HashError::Duplicate;
      }
      i = (i + 1) % Rows;
    } while (i != oi);
    return HashError::Full;
  }

  IndexR remove(KeyType _key) noexcept {
    auto oi = hash(_key);
    auto i = oi;
    do {
      if (!table[i].is_used && !table[i].is_deleted) {
        return HashError::NotFound;
      } else if (table[i].is_used && table[i].key == _key) {
        table[i].is_deleted = true;
        size--;
        return i;
      }
      i = (i + 1) % Rows;
    } while (i != oi);
    return HashError::NotFound;
  }

  KeyType operator[](tbl_row_t row) const noexcept { return table[row].key; }

  bool is_valid(tbl_row_t row) const noexcept {
    return table[row].is_used && !table[row].is_deleted;
  }

 protected:
  inline tbl_row_t hash(KeyType key) const noexcept { return key % Rows; }

  std::array<Entry, Rows> table;

 public:
  tbl_row_t size;
};

}  // namespace PawnDB

#endif  // PAWNDB_HASH_H
