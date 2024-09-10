#ifndef FIXED_STRING_TYPE_H
#define FIXED_STRING_TYPE_H

#include <pawndb/basic_data.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>

namespace PawnDB {

  /**
   * @brief A example fixed size string class, POD compatible.
   * - It has trivial destructor
   * - It has trivial copy constructor
   * - It has trivial copy assignment constructor
   */
  template <std::size_t str_length>
  class FixedString : public BasicData<FixedString<str_length>> {
  public:
    FixedString() : byte_array({}), length(0) { checksum = check_sum(); }

    // With static_assert()
    template <std::size_t N>
    explicit FixedString(const char (&s)[N]) : byte_array({}), length(N - 1) {
      static_assert(N < str_length, "String literal exceeds given size!");
      static_assert(N != 0, "String literal cannot be 0 size!");
      std::copy(s, s + N, byte_array.data());
      checksum = check_sum();
    }

    explicit FixedString(std::string&& str) : byte_array({}), length(str.length()) {
      if (str.length() > str_length) {
        throw std::out_of_range("Input str length is longer than given size.");
      }
      std::copy(str.begin(), str.end(), byte_array.data());
      checksum = check_sum();
    }

    std::uint32_t check_sum() const {
      std::uint32_t temp_checksum = 0;
      for (auto c : byte_array) {
        temp_checksum += c;
      }
      return temp_checksum;
    }

    // Required hash function
    std::size_t user_hash() const { return checksum; }

    // Require user_verify
    bool user_verify() const { return check_sum() == this->checksum; }

    // Required equal function
    bool user_eq(const FixedString<str_length>& str) const {
      return this->to_std_string() == str.to_std_string();
    }

    // Required not equal function
    bool user_neq(const FixedString<str_length>& str) const {
      return this->to_std_string() != str.to_std_string();
    }

    std::string user_print() const { return std::string(this->byte_array.data(), length); }

    /**
     * @brief Used to get a read-only std::string_view object from FixedString
     *
     * @return std::string_view
     */
    std::string_view to_std_string() const {
      return std::string_view(this->byte_array.data(), length);
    }

    std::array<char, str_length> byte_array;
    std::size_t length;

  private:
    volatile std::uint32_t checksum;
  };

}  // namespace PawnDB

#endif