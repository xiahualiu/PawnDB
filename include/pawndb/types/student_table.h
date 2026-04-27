/**
 * @file student_table.h
 * @author Xiahua Liu @xiahualiu
 * @brief Thread-safe associative container table type. It stores a fixed
 * number of key value pairs.
 * @version 0.1
 * @date 2025-01-02
 *
 * An example resource table type stored in a buffer pool slot. The primary
 * goal is to maintain thread safety. Each tuple slot contains a key, value,
 * and lock ID. Lookup key is computed using `tp_id`. This table has a
 * fixed size BUFFER_ROWS. The client is responsible for maintaining its
 * own mapping of entries to buffers.
 *
 * @copyright MIT License
 *
 */

#ifndef PAWNDB_TYPES_STUDENT_TABLE_H
#define PAWNDB_TYPES_STUDENT_TABLE_H

#include <sys/types.h>

#include <array>
#include <condition_variable>
#include <cstddef>
#include <cstring>
#include <mutex>
#include <string>

#include "pawndb/params.h"
#include "pawndb/result.h"

namespace PawnDB {

/** @brief Student table operation error codes */
enum class StudentTableError {
  None,    /**< Operation successful */
  NotFound /**< Entry not found */
};

/** @brief Tuple table operation error codes */
enum class TupleTableError {
  None,    /**< Operation successful */
  Full,    /**< Table at capacity */
  Timeout, /**< Lock wait timeout */
  NotFound /**< Entry not found */
};

/** @brief Serializer error codes */
enum class SerializerError {
  None,         /**< No error */
  ExceededWidth /**< Data exceeds buffer width */
};

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
class StudentTuple {
 public:
  using key_t = tbl_row_t;                       /**< Key type alias */
  constexpr static std::size_t NAME_LENGTH = 32; /**< Fixed name length */

  /** @brief Result type for serialization */
  using serial_r = Result<void, SerializerError>;

  // Constexpr Constructors
  constexpr StudentTuple() noexcept
      : checksum_(0), tickstamp_(0), name_({0}), age_(0), key_(0) {}

  /** @brief Construct student record
   *  @param _name Student name
   *  @param _age Student age
   *  @param _key Entry key */
  StudentTuple(std::string _name, const std::uint8_t _age,
               const tbl_row_t _key) noexcept;

  // Copyable
  StudentTuple(const StudentTuple& other) noexcept;
  StudentTuple& operator=(const StudentTuple& other) noexcept;

  /** @brief Set entry checksum */
  void set_checksum_() noexcept;

  /** @brief Validate checksum */
  bool val_checksum_() const noexcept;

  /** @brief Set entry timestamp */
  void set_tickstamp_(const tick_t tickstamp) noexcept;

  /** @brief Get entry timestamp */
  tick_t read_tickstamp_() const noexcept;

  /** @brief Get entry key */
  tbl_row_t key() const noexcept;

  /** @brief Serialize entry */
  serial_r serialize_(buf_t& buffer, std::size_t offset) const noexcept;

  /** @brief Deserialize entry */
  serial_r deserialize_(const buf_t& buffer, std::size_t offset) noexcept;

  /** @brief Compute hash */
  std::size_t hash_() const noexcept;

  /** @brief Create deep copy */
  StudentTuple copy_() const noexcept;

  /** @brief Copy from another tuple */
  void copy_from_(const StudentTuple& other) noexcept;

  /** @brief Compare equality */
  bool equals_(const StudentTuple& other) const noexcept;

  /** @brief Test helper: get age */
  std::uint8_t _test_age() const noexcept {
    return age_;
  }

  /** @brief Test helper: get name */
  std::string _test_name() const noexcept {
    return std::string(name_.data());
  }

 private:
  /** @brief Compute checksum */
  cksum_t compute_checksum_() const noexcept;

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
class StudentTable {
 public:
  using ttable_r = Result<void, TupleTableError>;

  constexpr static std::size_t Rows = 10;

  using key_t = tbl_row_t;
  using tuple_t = StudentTuple;

  /** @brief Result type for table operations */
  using table_r = Result<StudentTuple&, StudentTableError>;

  /** @brief Entry in student table */
  struct Entry {
    StudentTuple tuple_;
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

  /** @brief Insert new student record
   *  @param _tuple Tuple to insert
   *  @return Result containing inserted entry or error */
  table_r insert_(const tuple_t& _tuple) noexcept;

  /** @brief Search for student by key
   *  @param _key Key to search for
   *  @return Result containing found entry or error */
  table_r search_(const key_t& _key) noexcept;

  /** @brief Remove student record
   *  @param _key Key of record to remove
   *  @return Error status */
  StudentTableError remove_(const key_t& _key) noexcept;

  /** @brief Clear the student table */
  void clear_() noexcept;

  /** @brief Update student record
   *  @param _tuple Tuple with updated values
   *  @return Error status */
  StudentTableError write_(const tuple_t& _tuple) noexcept;

  /** @brief Wait for shared access */
  ttable_r wait_shared_() noexcept;

  /** @brief Wait for exclusive access */
  ttable_r wait_exclusive_() noexcept;

  /** @brief Promote lock mode
   *  @param _key Key of entry to promote
   *  @return Error status */
  TupleTableError promote_(const key_t& _key) noexcept;

  /** @brief Release all locks on entry
   *  @param _key Key of entry to release */
  void release_(const key_t& _key) noexcept;

  /** @brief Signal shared lock available */
  void notify_shared_() noexcept;

  /** @brief Signal exclusive lock available */
  void notify_exclusive_() noexcept;

  /** @brief Signal table not empty */
  void notify_not_empty_() noexcept;

  /** @brief Signal table not full */
  void notify_not_full_() noexcept;

  /** @brief Get number of entries */
  std::size_t size_() const noexcept;

  /** @brief Check if empty */
  bool empty_() const noexcept;

  /** @brief Check if full */
  bool full_() const noexcept;

  /** @brief Same as search but return the whole entry */
  Result<Entry&, StudentTableError> _test_get_entry(const key_t& _key) noexcept;

  /** @brief Get number of shared locks available */
  tbl_row_t _test_s_avail_cnt() const noexcept;

  /** @brief Get number of exclusive locks available */
  tbl_row_t _test_x_avail_cnt() const noexcept;

 private:
  /** @brief Set deleted flag on an entry */
  void set_deleted_(bool val, std::size_t idx) noexcept;

  /** @brief Seek an entry by key (internal) */
  table_r seek_(const key_t& _key) noexcept;

  /** @brief Find a slot for insertion (internal) */
  table_r find_(const StudentTuple& _tuple) noexcept;

  std::condition_variable s_available_; /**< SHARED lock CV */
  std::condition_variable x_available_; /**< Exclusive lock CV */
  std::condition_variable not_empty_;   /**< Not empty CV */
  std::condition_variable not_full_;    /**< Not full CV */
  std::mutex mtx_;                      /**< Thread safety */

  std::array<Entry, Rows> table_;

  tbl_row_t s_avail_cnt_; /**< Number of shared locks available */
  tbl_row_t x_avail_cnt_; /**< Number of exclusive locks available */
  std::size_t entry_count_;
  key_t next_key_;
};

}  // namespace PawnDB

#endif  // PAWNDB_TYPES_STUDENT_TABLE_H