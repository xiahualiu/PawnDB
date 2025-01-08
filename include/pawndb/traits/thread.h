#ifndef PAWNDB_TRAITS_THREAD_H
#define PAWNDB_TRAITS_THREAD_H

namespace PawnDB {

template <typename Derived>

class ThreadTrait {
 public:
  void start() noexcept { return static_cast<Derived*>(this)->trait_start(); }

  void stop() noexcept { static_cast<Derived*>(this)->trait_stop(); }

  bool is_running() const noexcept {
    return static_cast<const Derived*>(this)->trait_is_running();
  }
};

}  // namespace PawnDB

#endif
