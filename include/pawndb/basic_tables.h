#ifndef BASIC_TABLES_H
#define BASIC_TABLES_H

#include <array>
#include <cstddef>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>

namespace PawnDB {

  /**
   * @brief The base table class for implementing your own table class
   *
   * @tparam tuple_type: the row types
   * @tparam table_size: the max size of the table.
   */
  template <class tuple_type, std::size_t table_size>
  class BasicTable {
    /**
     * @brief Tuple must have validate function
     *
     */
    static_assert((std::is_member_function_pointer_v<decltype(&tuple_type::validate)>),
                  "Validate function is missing in the given tuple type");

  public:
    /**
     * @brief Get table size
     *
     * @return constexpr std::size_t
     */
    static constexpr std::size_t size() { return table_size; }
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

    /**
     * @brief Overload the [] operator so table can be used as a array.
     *
     * @param index The index
     * @return tuple_type&
     */
    inline tuple_type& operator[](std::size_t index) {
      if (index >= table_size)
        throw std::out_of_range("The index given is out of the table index range.");
      else
        return table[index];
    }

    inline void insert_at(std::size_t index, tuple_type& tmp) {
      if (index >= table_size)
        throw std::out_of_range("The index given is out of the table index range.");
      else
        table[index] = tmp;
    }

  protected:
    BasicTable() : table_mutex(), table({}), last_row(0) {}

  private:
    /**
     * @brief Table mutex, used in get_xxx_lock() functions
     *
     */
    std::shared_mutex table_mutex;
    std::array<tuple_type, table_size> table;
    std::size_t last_row;
  };

}  // namespace PawnDB

#endif