#include <iostream>
#include <unordered_set>

#include "pawndb/types/tuple_uid.h"

using namespace PawnDB;

int main() {
  std::cout << "Testing TupleUID class..." << std::endl;

  // Test default constructor
  TupleUID default_uid;
  if (default_uid.is_valid()) {
    std::cerr << "ERROR: Default UID should be invalid" << std::endl;
    return 1;
  }
  std::cout << "✓ Default constructor creates invalid UID" << std::endl;

  // Test parameterized constructor
  TupleUID uid1(1, 2, 1000);
  if (!uid1.is_valid()) {
    std::cerr << "ERROR: Constructed UID should be valid" << std::endl;
    return 1;
  }
  std::cout << "✓ Parameterized constructor creates valid UID" << std::endl;

  // Test getters
  if (uid1.table_id() != 1 || uid1.tuple_id() != 2 || uid1.transaction_id() != 1000) {
    std::cerr << "ERROR: Getters return incorrect values" << std::endl;
    return 1;
  }
  std::cout << "✓ Getters return correct values" << std::endl;

  // Test equality
  TupleUID uid2(1, 2, 1000);
  TupleUID uid3(1, 2, 1001);
  
  if (!(uid1 == uid2)) {
    std::cerr << "ERROR: Equal UIDs should compare equal" << std::endl;
    return 1;
  }
  
  if (uid1 == uid3) {
    std::cerr << "ERROR: Different UIDs should not compare equal" << std::endl;
    return 1;
  }
  std::cout << "✓ Equality comparison works correctly" << std::endl;

  // Test hash functionality
  std::unordered_set<TupleUID> uid_set;
  uid_set.insert(uid1);
  uid_set.insert(uid2);  // Should not increase size (same as uid1)
  uid_set.insert(uid3);  // Should increase size (different from uid1)
  
  if (uid_set.size() != 2) {
    std::cerr << "ERROR: Hash functionality not working correctly" << std::endl;
    return 1;
  }
  std::cout << "✓ Hash functionality works correctly" << std::endl;

  // Test comparison operators
  TupleUID uid4(1, 2, 999);  // Earlier transaction
  TupleUID uid5(2, 1, 1000); // Different table
  
  if (!(uid4 < uid1)) {
    std::cerr << "ERROR: Comparison operators not working correctly" << std::endl;
    return 1;
  }
  
  if (!(uid1 < uid5)) {
    std::cerr << "ERROR: Comparison operators not working correctly" << std::endl;
    return 1;
  }
  std::cout << "✓ Comparison operators work correctly" << std::endl;

  // Test string representation
  std::string str = uid1.to_string();
  std::string expected = "1:2:1000";
  if (str != expected) {
    std::cerr << "ERROR: String representation incorrect. Expected: " << expected 
              << ", Got: " << str << std::endl;
    return 1;
  }
  std::cout << "✓ String representation works correctly" << std::endl;

  // Test reset
  uid1.reset();
  if (uid1.is_valid()) {
    std::cerr << "ERROR: Reset should make UID invalid" << std::endl;
    return 1;
  }
  std::cout << "✓ Reset functionality works correctly" << std::endl;

  std::cout << "All tests passed!" << std::endl;
  return 0;
}
