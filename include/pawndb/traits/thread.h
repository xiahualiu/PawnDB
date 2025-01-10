#ifndef PAWNDB_TRAITS_THREAD_H
#define PAWNDB_TRAITS_THREAD_H

namespace PawnDB {

template <typename Derived>

/**
 * @brief CRTP base class for thread management
 * @tparam Derived The derived class implementing thread behavior
 *
 * Required implementations:
 * - trait_start()
 * - trait_stop()
 * - trait_is_running()
 */
class ThreadTrait {
 public:
  /** @brief Start thread execution */
  void start() noexcept { return static_cast<Derived*>(this)->trait_start(); }

  /** @brief Stop thread execution */
  void stop() noexcept { static_cast<Derived*>(this)->trait_stop(); }

  /**
   * @brief Check if thread is running
   * @return true if thread is running, false otherwise
   */
  bool is_running() const noexcept {
    return static_cast<const Derived*>(this)->trait_is_running();
  }
};

}  // namespace PawnDB

#endif
