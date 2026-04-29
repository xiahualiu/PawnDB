#include "pawndb/types/ret.h"

namespace PawnDB {

ret::ret(buf_ref&& _buffer, std::size_t _buffer_size,
         client_conn _conn) noexcept
    : buf_(std::move(_buffer), _buffer_size), conn_(_conn) {}

ret::ret(const ret& other) noexcept : buf_(other.buf_), conn_(other.conn_) {}

ret& ret::operator=(const ret& other) noexcept {
  buf_ = other.buf_;
  conn_ = other.conn_;
  return *this;
}

ret_buf& ret::buf() noexcept {
  return buf_;
}

const ret_buf& ret::buf() const noexcept {
  return buf_;
}

std::size_t ret::buf_sz() const noexcept {
  return buf_.get_buffer_size();
}

const client_conn& ret::conn() const noexcept {
  return conn_;
}

}  // namespace PawnDB
