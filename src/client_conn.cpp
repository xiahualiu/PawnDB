#include "pawndb/types/client_conn.h"

#include <cstring>

namespace PawnDB {

client_conn::client_conn(const sockaddr* addr, socklen_t addr_len) noexcept
    : addr_(), addr_len_(addr_len) {
  std::memcpy(&addr_, addr, addr_len);
}

client_conn::client_conn(const sockaddr_un& addr, socklen_t addr_len) noexcept
    : addr_(), addr_len_(addr_len) {
  std::memcpy(&addr_, &addr, addr_len);
}

ssize_t client_conn::reply(int server_fd, const void* data,
                           std::size_t size) const noexcept {
  return sendto(server_fd, data, size, 0,
                reinterpret_cast<const sockaddr*>(&addr_), addr_len_);
}

}  // namespace PawnDB
