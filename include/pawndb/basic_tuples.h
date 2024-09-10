#ifndef BASIC_TUPLES_H
#define BASIC_TUPLES_H

#include <cstddef>
#include <tuple>

#include "pawndb/basic_data.h"

namespace PawnDB {
  /**
   * @brief The base class for implementing your own tuple type.
   *
   * @tparam tuple_type the list of the tuple data types, must be derived from the BasicType
   * abstract class.
   */
  template <class T, class... tuple_types>
  class BasicTuple {
  public:
    /**
     * @brief Return how many items in the tuple type
     *
     * @return std::size_t the number of columns inside this typle
     */
    constexpr std::size_t length() const { return sizeof...(tuple_types); }

    /**
     * @brief Return a copy of the data object stored inside the tuple
     *
     * @tparam I n-th element inside the tuple
     * @return std::tuple_element<I, std::tuple<BasicData<tuple_types>...>>::type
     */
    template <std::size_t I>
    typename std::tuple_element<I, std::tuple<BasicData<tuple_types>...>>::type get()
        const noexcept {
      return static_cast<const T*>(this)->user_get_cp();
    }

    /**
     * @brief Return a lvalue ref of the data object stored inside the tuple
     *
     * @tparam I n-th element inside the tuple
     * @return std::tuple_element<I, std::tuple<BasicData<tuple_types>...>>::type
     */
    template <std::size_t I>
    typename std::tuple_element<I, std::tuple<BasicData<tuple_types>...>>::type& get() noexcept {
      return static_cast<T*>(this)->user_get_ref();
    }

    /**
     * @brief Return a rvalue ref of the data object stored inside the tuple
     *
     * @tparam I
     * @return std::tuple_element<I, std::tuple<BasicData<tuple_types>...>>::type&&
     */
    template <std::size_t I>
    typename std::tuple_element<I, std::tuple<BasicData<tuple_types>...>>::type&& get() noexcept {
      return static_cast<T*>(this)->user_get_mv();
    }

    /**
     * @brief Get the primary key of a tuple
     *
     * @return std::size_t
     */
    std::size_t hash() const { return static_cast<const T*>(this)->user_hash(); }

    /**
     * @brief Tuple must provide user_verify()
     *
     * @return true
     * @return false
     */
    bool verify() const { return recursive_verify<0, sizeof...(tuple_types)>(); }

  protected:
  private:
    /**
     * @brief Recursively verify all data inside a tuple
     *
     * @tparam I
     * @tparam Ts
     * @return true
     * @return false
     */
    template <std::size_t I, std::size_t Ts>
    bool recursive_verify() const {
      if constexpr (I >= Ts) {
        return true;
      } else if (!this->get<I>().verify()) {
        return false;
      } else {
        return recursive_verify<I + 1, Ts>();
      }
    }
  };

}  // namespace PawnDB

#endif