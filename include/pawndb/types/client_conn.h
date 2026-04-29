#ifndef PAWNDB_TYPES_CLIENT_CONN_H
#define PAWNDB_TYPES_CLIENT_CONN_H

#include <sys/socket.h>
#include <sys/un.h>

#include <cstddef>

namespace PawnDB {

/**
 * @brief Abstracted client connection descriptor
 *
 * Stores the client address in a protocol-agnostic sockaddr_storage so
 * callers (job, ret, reply paths) never need to know whether the underlying
 * transport is Unix-domain, TCP, UDP, or something else.
 *
 * Copy is trivial (sockaddr_storage is standard-layout), so client_conn
 * works inside fixed-size arrays and channels without heap allocation.
 */
class client_conn {
 public:
  /** @brief Empty connection (no client) */
  constexpr client_conn() noexcept : addr_(), addr_len_(0) {}

  /** @brief Construct from any socket address */
  client_conn(const sockaddr* addr, socklen_t addr_len) noexcept;

  /** @brief Convenience constructor for Unix-domain addresses */
  client_conn(const sockaddr_un& addr, socklen_t addr_len) noexcept;

  /** @brief Send reply back through the server socket */
  ssize_t reply(int server_fd, const void* data,
                std::size_t size) const noexcept;

 private:
  sockaddr_storage addr_; /**< Protocol-agnostic address storage */
  socklen_t addr_len_;    /**< Actual address length */
};

}  // namespace PawnDB

#endif
