#include <tuple>

template <class... tuple_type>
class Tuple {
public:
  template <class... t>
  Tuple() : list() {}

private:
  std::tuple<tuple_type...> list;
};