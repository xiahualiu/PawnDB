#ifndef PAWNDB_TABLE_H
#define PAWNDB_TABLE_H

#include <sys/types.h>

#include <array>
#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <string>

#include "pawndb/params.h"
#include "pawndb/traits/container.h"
#include "pawndb/traits/copy.h"
#include "pawndb/traits/eq.h"
#include "pawndb/traits/hash.h"
#include "pawndb/traits/record.h"
#include "pawndb/traits/record_table.h"
#include "pawndb/traits/serializer.h"
#include "pawndb/traits/sized.h"
#include "pawndb/traits/table.h"

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
class StudentRecord : public RecordTrait<StudentRecord>,
                      public SerializerTrait<StudentRecord>,
                      private CopyTrait<StudentRecord>,
                      public HashTrait<StudentRecord>,
                      public EqTrait<StudentRecord, StudentRecord> {
 public:
  using key_t = tbl_row_t;                       /**< Key type alias */
  constexpr static std::size_t NAME_LENGTH = 32; /**< Fixed name length */

  // Constexpr Constructors
  constexpr StudentRecord() noexcept
      : checksum_(0), tickstamp_(0), name_({0}), age_(0), key_(0) {}

  /** @brief Construct student record
   *  @param _name Student name
   *  @param _age Student age
   *  @param _key Entry key */
  StudentRecord(std::string _name, const std::uint8_t _age,
                const tbl_row_t _key) noexcept;

  // Copyable
  StudentRecord(const StudentRecord& other) noexcept;
  StudentRecord& operator=(const StudentRecord& other) noexcept;

  // TupleTrait Implementation
  /** @brief Set entry checksum */
  void trait_set_checksum() noexcept;

  /** @brief Validate checksum */
  bool trait_val_checksum() const noexcept;

  /** @brief Get entry key */
  tbl_row_t trait_key() const noexcept;

  // SerializerTrait Implementation
  /** @brief Serialize entry */
  serial_r trait_serialize(buffer_t& buffer, std::size_t offset) const noexcept;

  /** @brief Deserialize entry */
  serial_r trait_deserialize(const buffer_t& buffer,
                             std::size_t offset) noexcept;

  // HashTrait Implementation
  std::size_t trait_hash() const noexcept;

  // Test functions
  std::uint8_t _test_age() const noexcept {
    return age_;
  }

  std::string _test_name() const noexcept {
    return std::string(name_.data());
  }

  // EqTrait Implementation
  /** @brief Compare entries */
  bool trait_equals(const StudentRecord& other) const noexcept;

 private:
  /** @brief Compute checksum */
  cksum_t compute_checksum() const noexcept;

  volatile cksum_t checksum_;          /**< Entry checksum */
  tick_t tickstamp_;                   /**< Entry timestamp */
  std::array<char, NAME_LENGTH> name_; /**< Student name */
  std::uint8_t age_;                   /**< Student age */
  tbl_row_t key_;                      /**< Entry key */

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
class StudentTable : public TableTrait<StudentTable, StudentRecord>,
                     public RecordTableTrait<StudentTable, StudentRecord>,
                     public SizedTrait<StudentTable>,
                     public ContainerTrait<StudentTable> {
 public:
  constexpr static std::size_t Rows = 10;

  using key_t = tbl_row_t;
  using tuple_t = StudentRecord;

  /** @brief Entry in student table */
  struct Entry {
    StudentRecord tuple_;
    lk_t lock_;
    bool is_used_;
    bool is_deleted_;

    /** @brief Construct empty entry */
    constexpr Entry() noexcept
        : tuple_{}, lock_(0), is_used_(false), is_deleted_(false) {}
  };

  /** @brief Construct student table */
  StudentTable() noexcept;

  // Non-copyable
  StudentTable(const StudentTable& other) noexcept = delete;
  StudentTable& operator=(const StudentTable& other) noexcept = delete;

  // TableTrait Implementation
  /** @brief Insert new student record
   *  @param _tuple Tuple to insert
   *  @return Result containing inserted entry or error */
  table_r trait_insert(const tuple_t& _tuple) noexcept;

  /** @brief Search for student by key
   *  @param _key Key to search for
   *  @return Result containing found entry or error */
  table_r trait_search(const key_t& _key) noexcept;

  /** @brief Remove student record
   *  @param _key Key of record to remove
   *  @return Error status */
  TableError trait_remove(const key_t& _key) noexcept;

  /** @brief Update student record
   *  @param _tuple Tuple with updated values
   *  @return Error status */
  TableError trait_update(const tuple_t& _tuple) noexcept;

  // TupleTableTrait Implementation

  // Condition Variable Notifications
  /** @brief Signal shared lock available */
  void trait_notify_shared() noexcept;

  /** @brief Signal exclusive lock available */
  void trait_notify_exclusive() noexcept;

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

  /** @brief Same as search but return the whole entry */
  Result<Entry&, TableError> _test_get_entry(const key_t& _key) noexcept;

  /** @brief Get number of shared locks available */
  tbl_row_t _test_s_avail_cnt() const noexcept;

  /** @brief Get number of exclusive locks available */
  tbl_row_t _test_x_avail_cnt() const noexcept;

 private:
  std::condition_variable s_available_; /**< SHARED lock CV */
  std::condition_variable x_available_; /**< Exclusive lock CV */
  std::condition_variable not_empty_;   /**< Not empty CV */
  std::condition_variable not_full_;    /**< Not full CV */
  std::mutex mtx_;                      /**< Thread safety */

  std::array<Entry, Rows> table_;

  tbl_row_t s_avail_cnt_; /**< Number of shared locks available */
  tbl_row_t x_avail_cnt_; /**< Number of exclusive locks available */
  std::size_t size_;
  key_t next_key_;
};
}  // namespace PawnDB

#endif  // PAWNDB_TABLE_H
