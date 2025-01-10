
#ifndef PAWNDB_TRAITS_BUFFER_TABLE_H
#define PAWNDB_TRAITS_BUFFER_TABLE_H

#include "pawndb/result.h"

namespace PawnDB {

template <typename Derived, typename RefCountType>
class BufferManagerTrait {
 public:
  /**
   * @brief Buffer operation error codes
   */
  enum class BufferError {
    None,       /**< Operation successful */
    Full,       /**< No free buffers */
    OutOfRange, /**< Invalid buffer index */
    NotUsed     /**< Buffer not allocated */
  };

  using RequestR = Result<RefCountType, BufferError>;

  RequestR request() noexcept {
    return static_cast<Derived*>(this)->trait_request();
  }

 protected:
  BufferManagerTrait() = default;
  ~BufferManagerTrait() = default;
};

}  // namespace PawnDB

#endif  // PAWNDB_TRAITS_BUFFER_TABLE_H
