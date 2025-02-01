#ifndef PAWNDB_TRAITS_THREAD_H
#define PAWNDB_TRAITS_THREAD_H

namespace PawnDB {

/**
 * @brief CRTP base class for thread management
 * @tparam Derived The derived class implementing thread behavior
 *
 * Required implementations:
 * - trait_start()
 * - trait_stop()
 * - trait_is_running()
 */
template <typename Derived>
class ThreadTrait {
 public:
  /** @brief Start thread execution */
  void start() noexcept {
    return derived().trait_start();
  }

  /** @brief Stop thread execution */
  void stop() noexcept {
    derived().trait_stop();
  }

  /** @brief clear the thread unfinished job then join the thread */
  void join() noexcept {
    derived().trait_join();
  }

  /**
   * @brief Check if thread is running
   * @return true if thread is running, false otherwise
   */
  bool is_running() noexcept {
    return derived().trait_is_running();
  }

 protected:
  // Protected constructor and destructor
  ThreadTrait() = default;
  ~ThreadTrait() = default;

  // CRTP helpers
  Derived& derived() noexcept {
    return static_cast<Derived&>(*this);
  }

  const Derived& derived() const noexcept {
    return static_cast<const Derived&>(*this);
  }
};

}  // namespace PawnDB

#endif
