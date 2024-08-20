#ifndef BASE_TYPE_H
#define BASE_TYPE_H

#include <cstddef>

namespace PawnDB {

  /**
   * @brief Abstract parent class for all data types in PawnDB
   *
   * @tparam checksum_type checksum data type, preferable stdint 
   *
   * @note You MUST overload operator== and operator!= in the derived class
   *       It cannot be enforced in the base class until C++20 concept.
   */
  template <typename checksum_type>
  class Base_Type {
  public:
    /**
     * @brief Pure virtual function for getting checksum of the data type
     *
     * @return std::atomic_uint
     */
    virtual checksum_type check_sum() const = 0;

    /**
     * @brief Hash function for creating hash index table
     * 
     * @return std::size_t 
     */
    virtual std::size_t hash() const = 0;

    /**
     * @brief Used to verify checksum
     * 
     * @return true checksum is valid
     * @return false checksum is not valid
     */
    inline bool verify_checksum() const {
      return checksum == check_sum();
    }

    /**
     * @brief Used to update the checksum on db write
     * 
     */
    inline void update_checksum() {
      this->checksum = check_sum();
    }

  protected:
    checksum_type checksum;

  private:
  };

}  // namespace PawnDB

#endif