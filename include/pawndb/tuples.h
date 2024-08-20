#ifndef TUPLES_H
#define TUPLES_H

#include <tuple>

namespace PawnDB {

  template <class... tuple_type>
  class Tuple {
  public:
    /**
     * @brief Construct a new Tuple object
     * 
     * @tparam t list of the types
     */
    template <class... t>
    Tuple() : list() {}

  private:
    std::tuple<tuple_type...> list;
  };

}  // namespace PawnDB

#endif