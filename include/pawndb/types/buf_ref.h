#ifndef PAWNDB_TYPES_BUF_REF_H
#define PAWNDB_TYPES_BUF_REF_H

#include <cstddef>

#include "pawndb/params.h"

namespace PawnDB {

class buf_table;

/**
 * @brief Buffer reference wrapper
 */
class buf_ref {
 private:
  buf_table* table_;  /**< Owner table reference */
  std::size_t index_; /**< Buffer index */

 public:
  /** @brief Default constructor - creates invalid reference */
  constexpr buf_ref() noexcept : table_(nullptr), index_(0) {}

  /** @brief Constructor with table and index
   *  @param table Owner buffer table
   *  @param index Buffer index */
  buf_ref(buf_table* table, std::size_t index) noexcept;

  // Copyable
  buf_ref(const buf_ref& other) noexcept;
  buf_ref& operator=(const buf_ref& other) noexcept;

  // Movable
  buf_ref(buf_ref&& other) noexcept;
  buf_ref& operator=(buf_ref&& other) noexcept;
  ~buf_ref() noexcept;

  friend bool test_null(const buf_ref& ref);
  friend std::size_t test_index(const buf_ref& ref);

  /** @brief Get underlying buffer */
  buf_t& buffer() const noexcept;

 private:
  void release_ref_() noexcept;
};

}  // namespace PawnDB

#endif  // PAWNDB_TYPES_BUF_REF_H
