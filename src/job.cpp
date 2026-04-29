#include "pawndb/types/job.h"

namespace PawnDB {

job::job(buf_ref&& _buffer, std::size_t _buffer_size,
         client_conn _conn) noexcept
    : buf_(std::move(_buffer), _buffer_size), conn_(_conn) {}

job::job(const job& other) noexcept : buf_(other.buf_), conn_(other.conn_) {}

job& job::operator=(const job& other) noexcept {
  buf_ = other.buf_;
  conn_ = other.conn_;
  return *this;
}

job_buf& job::buf() noexcept {
  return buf_;
}

const job_buf& job::buf() const noexcept {
  return buf_;
}

std::size_t job::buf_sz() const noexcept {
  return buf_.get_buffer_size();
}

const client_conn& job::conn() const noexcept {
  return conn_;
}

}  // namespace PawnDB
