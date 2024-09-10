#ifndef U32_TYPE_H
#define U32_TYPE_H

#include <pawndb/basic_data.h>

#include <cstddef>
#include <cstdint>
#include <string>

namespace PawnDB {

  /**
   * @brief A example u32 type class, POD compatible.
   * - It has trivial destructor
   * - It has trivial copy constructor
   * - It has trivial copy assignment constructor
   */
  class U32Int : public BasicData<U32Int> {
  public:
    U32Int() : value(0u) { checksum = check_sum(); };
    U32Int(const std::uint32_t& v) : value(v) { checksum = check_sum(); };
    U32Int(U32Int&& i) = delete;

    // Required checksum function
    inline std::uint32_t check_sum() const { return value; }
    // Required hash function
    inline std::size_t user_hash() const { return value; }
    // Required == function
    bool user_eq(const U32Int& i) const { return value == i.value; }
    // Required != function
    bool user_neq(const U32Int& i) const { return value != i.value; }
    // Required verify function
    bool user_verify() const { return checksum == check_sum(); }
    // Required print function
    std::string user_print() const { return std::to_string(value); }

    bool operator>(const U32Int& i) const { return value > i.value; }
    bool operator<(const U32Int& i) const { return value < i.value; }

    std::uint32_t value;

  private:
    volatile std::uint32_t checksum;
  };

}  // namespace PawnDB

#endif