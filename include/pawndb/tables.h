#ifndef TABLES_H
#define TABLES_H

#include <cstddef>
#include <mutex>
#include <shared_mutex>

namespace PawnDB {

  template <class tuple_type, std::size_t table_size>
  class Table {
  public:
    // store table size information
    static constexpr std::size_t length = table_size;

    Table() : table_mutex(), list() {}

    /**
     * @brief Get a shared lock object from this table
     *        May block the request thread
     *
     * @return std::shared_lock<std::shared_mutex>
     */
    inline std::shared_lock<std::shared_mutex> get_shared_lock() {
      return std::shared_lock(this->table_mutex);
    }

    /**
     * @brief Get a unique lock object from this table
     *        May block the request thread
     *
     * @return std::shared_lock<std::shared_mutex>
     */
    inline std::unique_lock<std::shared_mutex> get_exclusive_lock() {
      return std::unique_lock(this->table_mutex);
    }

  private:
    // Table mutex, can be acquired in shared or exclusive mode.
    std::shared_mutex table_mutex;
    tuple_type list[table_size];
  };

}  // namespace PawnDB

#endif