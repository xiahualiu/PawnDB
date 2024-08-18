#ifndef TABLES_H
#define TABLES_H

#include <cstddef>

namespace PawnDB {

  template <class tuple_type, std::size_t table_size>
  class Table {
  public:
    Table() : list() {}

  private:
    static constexpr std::size_t length = table_size;
    tuple_type list[table_size];
  };

}  // namespace PawnDB

#endif