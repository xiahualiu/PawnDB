#ifndef PAWNDB_TABLE_H
#define PAWNDB_TABLE_H

#include <sys/types.h>

#include <array>
#include <condition_variable>
#include <cstddef>
#include <mutex>

#include "pawndb/params.h"
#include "pawndb/traits/container.h"
#include "pawndb/traits/copy.h"
#include "pawndb/traits/hash.h"
#include "pawndb/traits/hash_table.h"
#include "pawndb/traits/serializer.h"
#include "pawndb/traits/sized.h"
#include "pawndb/traits/tuple.h"
#include "pawndb/traits/tuple_table.h"

namespace PawnDB {

/**
 * @brief Entry in student table storing student records
 *
 * Features:
 * - Fixed-size name storage
 * - Checksum validation
 * - Timestamp tracking
 * - Lock management
 * - Hash-based lookup
 */
class StudentTableEntry : public TupleTrait<StudentTableEntry>,
                          public SerializerTrait<StudentTableEntry>,
                          public CopyTrait<StudentTableEntry>,
                          public HashTrait<StudentTableEntry> {
 public:
  using key_t = tbl_row_t;                       /**< Key type alias */
  constexpr static std::size_t NAME_LENGTH = 32; /**< Fixed name length */

  // Constexpr Constructors
  constexpr StudentTableEntry() noexcept
      : checksum_(0),
        tickstamp_(0),
        name_({0}),
        age_(0),
        key_(0),
        lock_(0),
        is_used_(false),
        is_deleted_(false) {}

  /** @brief Copy constructor */
  StudentTableEntry(const StudentTableEntry& other) noexcept;

  /** @brief Copy assignment */
  StudentTableEntry& operator=(const StudentTableEntry& other) noexcept;

  // Not movable
  StudentTableEntry(StudentTableEntry&& other) noexcept = delete;
  StudentTableEntry& operator=(StudentTableEntry&& other) noexcept = delete;

  // TupleTrait Implementation
  /** @brief Set entry checksum */
  void trait_set_checksum() noexcept;

  /** @brief Validate checksum */
  bool trait_val_checksum() const noexcept;

  /** @brief Set entry timestamp */
  void trait_set_tickstamp(const tick_t tickstamp) noexcept;

  /** @brief Get entry timestamp */
  tick_t trait_read_tickstamp() const noexcept;

  // SerializerTrait Implementation
  /** @brief Serialize entry */
  serial_r trait_serialize(BufferRef buffer, std::size_t offset) const noexcept;

  /** @brief Deserialize entry */
  serial_r trait_deserialize(BufferRef buffer, std::size_t offset) noexcept;

  // CopyTrait Implementation
  /** @brief Clone entry */
  StudentTableEntry trait_clone() const noexcept;

  /** @brief Copy entry */
  void trait_copy(const StudentTableEntry& other) noexcept;

  // HashTrait Implementation
  std::size_t trait_hash() const noexcept;

 private:
  /** @brief Compute checksum */
  cksum_t compute_checksum() const noexcept;

  volatile cksum_t checksum_;          /**< Entry checksum */
  tick_t tickstamp_;                   /**< Entry timestamp */
  std::array<char, NAME_LENGTH> name_; /**< Student name */
  std::uint8_t age_;                   /**< Student age */
  tbl_row_t key_;                      /**< Entry key */
  lk_t lock_;                          /**< Entry lock */
  bool is_used_;                       /**< Usage flag */
  bool is_deleted_;                    /**< Deletion flag */

  friend class StudentTable;
};

/**
 * @brief Fixed-size table storing student records
 *
 * Features:
 * - O(1) hash-based lookup
 * - Checksum validation
 * - Concurrency control
 */
class StudentTable : public HashTableTrait<StudentTable, StudentTableEntry>,
                     public TupleTableTrait<StudentTable, StudentTableEntry>,
                     public SizedTrait<StudentTable>,
                     public ContainerTrait<StudentTable> {
  constexpr static std::size_t Rows = 10;

 public:
  using key_type = tbl_row_t;
  using entry_type = StudentTableEntry;

  StudentTable() noexcept : table_(), size_(0), tuple_key_(0) {}

  constexpr static std::size_t trait_id() noexcept {
    return 1;
  }

  // HashTableTrait Implementation
  /** @brief Insert new student record
   *  @param _entry Entry to insert
   *  @return Result containing inserted entry or error */
  table_r trait_insert(const entry_type& _entry) noexcept;

  /** @brief Search for student by key
   *  @param _key Key to search for
   *  @return Result containing found entry or error */
  table_r trait_search(const key_type& _key) noexcept;

  /** @brief Remove student record
   *  @param _key Key of record to remove
   *  @return Error status */
  TableError trait_remove(const key_type& _key) noexcept;

  /** @brief Update student record
   *  @param _entry Entry with updated values
   *  @return Error status */
  table_r trait_write(const entry_type& _entry) noexcept;

  // TupleTableTrait Implementation
  /** @brief Wait for shared access */
  ttable_r trait_wait_shared() noexcept;

  /** @brief Wait for exclusive access */
  ttable_r trait_wait_exclusive() noexcept;

  /** @brief Promote lock mode
   *  @param _key Key of entry to promote
   *  @return Error status */
  TupleTableError trait_promote(const key_type& _key) noexcept;

  /** @brief Release all locks on entry
   *  @param _key Key of entry to release */
  void trait_release(const key_type& _key) noexcept;

  // Condition Variable Notifications
  /** @brief Signal shared lock available */
  void trait_notify_s() noexcept;

  /** @brief Signal exclusive lock available */
  void trait_notify_x() noexcept;

  /** @brief Signal table not empty */
  void trait_notify_not_empty() noexcept;

  /** @brief Signal table not full */
  void trait_notify_not_full() noexcept;

  // Container Implementation
  /** @brief Get number of entries */
  std::size_t trait_size() const noexcept;

  /** @brief Check if empty */
  bool trait_empty() const noexcept;

  /** @brief Check if full */
  bool trait_full() const noexcept;

 private:
  std::condition_variable s_available_; /**< Shared lock CV */
  std::condition_variable x_available_; /**< Exclusive lock CV */
  std::condition_variable not_empty_;   /**< Not empty CV */
  std::condition_variable not_full_;     /**< Not full CV */
  std::mutex mtx_;                      /**< Thread safety */

  std::array<StudentTableEntry, Rows> table_;

  std::size_t size_;
  key_t tuple_key_;
};
}  // namespace PawnDB

#endif  // PAWNDB_TABLE_H
