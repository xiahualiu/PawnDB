/**
 * @file hash.h
 * @brief Implementation of a fixed-size hash table data structure
 * @version 0.1
 * @date 2025-01-02
 *
 * This hash table implementation provides:
 * - Fixed size allocation with no dynamic memory
 * - Linear probing for collision resolution
 * - O(1) average case lookup/insert/delete
 * - Type safety through templates
 *
 * @copyright MIT License
 */

#ifndef PAWNDB_HASH_H
#define PAWNDB_HASH_H

#include <array>

#include "pawndb/params.h"
#include "pawndb/result.h"

namespace PawnDB {

/**
 * @brief Enumeration of possible hash table errors.
 */
enum class HashError {
  None,      /**< No error */
  NotFound,  /**< Key not found */
  Full,      /**< Hash table is full */
  Duplicate, /**< Duplicate key */
};

/**
 * @brief Fixed-size hash table with linear probing
 *
 * @tparam KeyType The key type (must be integral)
 * @tparam Rows Maximum number of entries
 *
 * This hash table uses:
 * - Linear probing for collision resolution
 * - Lazy deletion with tombstone markers
 * - Static allocation with std::array
 */
template <class KeyType, tbl_row_t Rows>
class Hash {
  static_assert(std::is_integral_v<KeyType>,
                "KeyType must be an integral type!");
  static_assert(Rows > 0, "Rows must be greater than 0!");

 public:
  /**
   * @brief Entry in the hash table
   *
   * Each entry contains:
   * - A key value
   * - Usage flag for occupied slots
   * - Deletion flag for tombstone markers
   */
  struct Entry {
    KeyType key;     /**< The key of the entry */
    bool is_used;    /**< Flag indicating if the entry is used */
    bool is_deleted; /**< Flag indicating if the entry is deleted */
  };

  /**
   * @brief Constructs a new Hash object.
   */
  constexpr Hash() noexcept : table(), size() {}

  /**
   * @brief Checks if the hash table is empty.
   *
   * @return true if the hash table is empty, false otherwise.
   */
  inline bool empty() const noexcept { return size == 0; }

  /**
   * @brief Checks if the hash table is full.
   *
   * @return true if the hash table is full, false otherwise.
   */
  inline bool full() const noexcept { return size == Rows; }

  using IndexR = Result<tbl_row_t, HashError>;

  /**
   * @brief Searches for a key in the hash table
   *
   * Uses linear probing to find key, handling:
   * - Empty slots (not found)
   * - Deleted slots (continue search)
   * - Occupied slots (check key match)
   *
   * @param _key Key to search for
   * @return Index if found, NotFound error if not present
   */
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

  /**
   * @brief Inserts a key into the hash table
   *
   * Finds first available slot using linear probing.
   * Returns error if table full or key exists.
   *
   * @param _key Key to insert
   * @return Index where inserted, or error code
   */
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

  /**
   * @brief Removes a key from the hash table
   *
   * Uses lazy deletion by marking slot as deleted.
   * Returns error if key not found.
   *
   * @param _key Key to remove
   * @return Success/failure status
   */
  IndexR remove(KeyType _key) noexcept {
    auto oi = hash(_key);
    auto i = oi;
    do {
      if (!table[i].is_used && !table[i].is_deleted) {
        return HashError::NotFound;
      } else if (table[i].is_used && table[i].key == _key) {
        table[i].is_used = false;
        table[i].is_deleted = true;
        size--;
        return i;
      }
      i = (i + 1) % Rows;
    } while (i != oi);
    return HashError::NotFound;
  }

  /**
   * @brief Gets the key at a specific row in the hash table.
   *
   * @param row The row to get the key from.
   * @return KeyType The key at the specified row.
   */
  KeyType operator[](tbl_row_t row) const noexcept { return table[row].key; }

  /**
   * @brief Checks if a row in the hash table is valid.
   *
   * @param row The row to check.
   * @return true if the row is valid, false otherwise.
   */
  bool is_valid(tbl_row_t row) const noexcept {
    return table[row].is_used && !table[row].is_deleted;
  }

 protected:
  /**
   * @brief Computes initial hash index for key
   *
   * Simple modulo hash function.
   *
   * @param key Input key
   * @return Initial table index
   */
  tbl_row_t hash(KeyType key) const noexcept { return key % Rows; }

  std::array<Entry, Rows> table;
  tbl_row_t size;
};

}  // namespace PawnDB

#endif  // PAWNDB_HASH_H
