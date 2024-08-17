#include <cstddef>

template <class tuple_type, std::size_t table_size>
class Table {
public:
  template <class t>
  Table(std::size_t &&s) : length(s), list() {}

private:
  const std::size_t length;
  tuple_type list[table_size];
};