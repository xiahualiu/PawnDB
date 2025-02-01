#ifndef PAWNDB_TRAITS_WORKER_CONTEXT_H
#define PAWNDB_TRAITS_WORKER_CONTEXT_H

#include "pawndb/params.h"
#include "pawndb/schema/demo.h"
#include "pawndb/types/job_channel.h"
#include "pawndb/types/ret_channel.h"

namespace PawnDB {

/**
 * @brief CRTP interface for worker context implementations
 * @tparam Derived The derived context class
 */
template <typename Derived>
class WorkerContextTrait {
 public:
  /** @brief Get job channel */
  JobChannel& job_ch() const noexcept {
    return derived().trait_job_ch();
  }

  /** @brief Get return channel */
  RetChannel& ret_ch() const noexcept {
    return derived().trait_ret_ch();
  }

  /** @brief Get database instance */
  Database& db() const noexcept {
    return derived().trait_db();
  }

  /** @brief Get transaction ID */
  txn_id_t txn_id() const noexcept {
    return derived().trait_txn_id();
  }

  /** @brief Get server socket descriptor */
  int fd() const noexcept {
    return derived().trait_fd();
  }

 protected:
  // Protected constructor and destructor
  WorkerContextTrait() = default;
  ~WorkerContextTrait() = default;

  // CRTP helpers
  Derived& derived() noexcept {
    return static_cast<Derived&>(*this);
  }

  const Derived& derived() const noexcept {
    return static_cast<const Derived&>(*this);
  }
};

}  // namespace PawnDB

#endif  // PAWNDB_TRAITS_WORKER_CONTEXT_H
