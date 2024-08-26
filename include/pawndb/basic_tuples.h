#ifndef BASIC_TUPLES_H
#define BASIC_TUPLES_H

#include <cstddef>
#include <tuple>
#include <type_traits>

namespace PawnDB {

  namespace CheckEqual {
    struct NoEqual {};
    template <typename T, typename Arg>
    NoEqual operator==(const T &, const Arg &);
    template <typename T, typename Arg = T>
    struct EqualExists {
      static constexpr bool value
          = !std::is_same<decltype(std::declval<const T &>() == std::declval<const Arg &>()),
                          NoEqual>::value;
    };
  }  // namespace CheckEqual

  /**
   * @brief The base class for implementing your own tuple type.
   *
   * @tparam tuple_type the list of the tuple data types, must be derived from the BasicType
   * abstract class.
   */
  template <class... tuple_types>
  class BasicTuple {
    /**
     * @brief Make sure all data types inside the tuple have hash and checksum functions
     *
     */
    static_assert((std::is_member_function_pointer_v<decltype(&tuple_types::hash)> && ...),
                  "Hash function is missing in one of the data types");
    static_assert(
        (std::is_same_v<decltype(std::declval<tuple_types>().hash()), std::size_t> && ...),
        "Hash function must return std::size_t type");
    static_assert(
        (std::is_member_function_pointer_v<decltype(&tuple_types::verify_checksum)> && ...),
        "Verify checksum function is missing in one of the data types");
    static_assert(
        (std::is_same_v<decltype(std::declval<tuple_types>().verify_checksum()), bool> && ...),
        "Verify checksum function must return bool type");
    static_assert((CheckEqual::EqualExists<tuple_types>::value && ...),
                  "Data types in tuple must have == operator defined.");
    /**
     * @brief Make sure we can copy data from/to these data types
     *
     */
    static_assert((std::is_copy_assignable_v<tuple_types> && ...),
                  "Data types in tuple must can be copy assigned");
    static_assert((std::is_copy_constructible_v<tuple_types> && ...),
                  "Data types in tuple must can be copy constructed");

  public:
    /**
     * @brief Copy construct a new Basic Tuple object
     *
     * @param tuple another tuple object
     */
    inline BasicTuple(const BasicTuple &tuple) : list(tuple.list) {}

    /**
     * @brief Return how many items in the tuple type
     *
     * @return std::size_t the number of columns inside this typle
     */
    inline std::size_t length() { return sizeof...(tuple_types); }

    std::tuple<tuple_types...> list;

  protected:
    /**
     * @brief Construct a new Tuple object
     *
     * @tparam t list of the types
     */
    inline BasicTuple() : list() {}

  private:
  };

}  // namespace PawnDB

#endif