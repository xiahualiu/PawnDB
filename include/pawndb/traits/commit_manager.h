#ifndef PAWNDB_TRAITS_COMMIT_MANAGER_H
#define PAWNDB_TRAITS_COMMIT_MANAGER_H

namespace PawnDB {

/**
 * @brief Commit operation error codes
 */
enum class CommitError {
  None,          /**< Operation successful */
  Full,          /**< Commit table full */
  InvalidOp,     /**< Invalid operation */
  InvalidKey,    /**< Invalid key */
  AlreadyExists, /**< Commit already exists */
  Unknown        /**< Unknown Error */
};

/**
 * @brief CRTP interface for commit manager implementations
 * @tparam Derived The derived commit manager class
 */
template <typename Derived, typename CommitEntryType>
class CommitManagerTrait {
 public:
  /**
   * @brief Add new commit entry
   * @param _key Key for commit
   * @return Result indicating success/error
   */
  CommitError add_commit(const CommitEntryType& _key) noexcept {
    return static_cast<Derived*>(this)->trait_add_commit(_key);
  }

 protected:
  CommitManagerTrait() = default;
  ~CommitManagerTrait() = default;
};

}  // namespace PawnDB

#endif  // PAWNDB_TRAITS_COMMIT_MANAGER_H
