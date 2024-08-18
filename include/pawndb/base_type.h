#ifndef BASE_TYPE_H
#define BASE_TYPE_H

#include <cstdint>

namespace PawnDB {

  /**
   * @brief Abstract parent class for all data types in PawnDB
   * It provides:
   * - checksum hash function for data integrity check
   */
  class Base_Type {
  public:
    /**
     * @brief Pure virtual function for getting checksum of the data type
     *
     * @return std::atomic_uint
     */
    virtual std::uint32_t check_sum() const = 0;

  protected:
    std::uint32_t checksum;

  private:
  };

}  // namespace PawnDB

#endif