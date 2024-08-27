#ifndef STUDENT_TABLE_H
#define STUDENT_TABLE_H

#include <pawndb/basic_tables.h>
#include <pawndb/user/student_tuple.h>

namespace PawnDB {

  class StudentTable : public BasicTable<StudentTuple, 100> {
  public:
  private:
  };

}  // namespace PawnDB

#endif