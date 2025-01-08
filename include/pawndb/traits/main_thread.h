#ifndef PAWNDB_TRAITS_MAIN_THREAD_H
#define PAWNDB_TRAITS_MAIN_THREAD_H

#include <type_traits>

#include "pawndb/result.h"

namespace PawnDB {
/**
 * @brief Main thread operation error codes
 */
enum class MainError {
  None,        /**< Operation successful */
  StartFailed, /**< Thread failed to start */
  SocketError, /**< Socket operation failed */
};

/**
 * @brief CRTP interface for main thread implementations
 * @tparam Derived Class implementing the main thread interface
 * @tparam Context Type containing thread context data
 *
 * Required implementations:
 * - MainError trait_start(const context_type&)
 * - void trait_stop()
 * - bool trait_is_running() const
 */
template <typename Derived, typename Context>
class MainThread {
 public:
  using context_type = Context;
  using MainR = Result<void, MainError>;

  /**
   * @brief Start main thread execution
   * @param _ct Thread context data
   * @return MainError None or error code
   */
  MainError start(const context_type& _ct) noexcept {
    return static_cast<Derived*>(this)->trait_start(_ct);
  }

  /**
   * @brief Stop main thread execution
   */
  void stop() noexcept { static_cast<Derived*>(this)->trait_stop(); }

  /**
   * @brief Check if thread is running
   * @return true if thread is active
   */
  bool is_running() const noexcept {
    return static_cast<const Derived*>(this)->trait_is_running();
  }

 protected:
  MainThread() = default;
  ~MainThread() = default;

  static_assert(
      std::is_member_function_pointer_v<decltype(&Derived::trait_start)>,
      "Must implement MainError trait_start(const context_type&)");
  static_assert(
      std::is_member_function_pointer_v<decltype(&Derived::trait_stop)>,
      "Must implement void trait_stop()");
  static_assert(std::is_member_function_pointer_v<
                    decltype(&Derived::trait_process_request)>,
                "Must implement MainError trait_process_request()");
  static_assert(std::is_member_function_pointer_v<
                    decltype(&Derived::trait_manage_workers)>,
                "Must implement MainError trait_manage_workers()");
  static_assert(
      std::is_member_function_pointer_v<decltype(&Derived::trait_is_running)>,
      "Must implement bool trait_is_running() const");
};

}  // namespace PawnDB

#endif
