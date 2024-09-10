#ifndef BASIC_DATA_H
#define BASIC_DATA_H

#include <cstddef>
#include <string>

namespace PawnDB {

  template <typename T>
  class BasicData {
  public:
    /**
     * @brief user_hash() is required for derived data types
     *
     */
    std::size_t hash() const { return static_cast<const T*>(this)->user_hash(); }

    /**
     * @brief user_verify() function is required for derived data types
     *
     */
    bool verify() const { return static_cast<const T*>(this)->user_verify(); }

    /**
     * @brief user_print() function is required for derived data types
     *
     */
    std::string print() const { return static_cast<const T*>(this)->user_print(); }
  };

  /**
   * @brief Equal function is required for derived data types
   *
   * @tparam T Derived data types
   * @param op1 oprand 1
   * @param op2 oprand 2
   * @return true equal
   * @return false not equal
   */
  template <class T>
  bool operator==(BasicData<T> const& op1, BasicData<T> const& op2) {
    return static_cast<const T&>(op1).user_eq(static_cast<const T&>(op2));
  }

  /**
   * @brief Not Equal function is required for derived data types
   *
   * @tparam T Derived data types
   * @param op1 oprand 1
   * @param op2 oprand 2
   * @return true equal
   * @return false not equal
   */
  template <class T>
  bool operator!=(BasicData<T> const& op1, BasicData<T> const& op2) {
    return static_cast<const T&>(op1).user_neq(static_cast<const T&>(op2));
  }
}  // namespace PawnDB

#endif