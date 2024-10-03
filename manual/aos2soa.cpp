#include <iostream>
#include <vector>

namespace mds {
template <typename T>
class vector {
private: //
  std::vector<double> _x, _y, _z, _value;
  struct aos_view {
    const double &x, &y, &z, &value;
  };

public:
  size_t size() const { return _x.size(); }

  void push_back(T elem) & {
    _x.push_back(elem.x);
    _y.push_back(elem.y);
    _z.push_back(elem.z);
    _value.push_back(elem.value);
  }

  aos_view operator[](std::size_t idx) const { return aos_view(_x[idx], _y[idx], _z[idx], _value[idx]); }
};
} // namespace mds

// dummy
struct data {
  double x, y, z, value;
};

int main() {
  mds::vector<data> maos;

  data e1 = {0, 1, 2, 3};
  data e2 = {4, 5, 6, 7};
  data e3 = {8, 9, 10, 11};

  maos.push_back(e1);
  maos.push_back(e2);
  maos.push_back(e3);

  std::cout << "maos.size = " << maos.size() << "\n";
  for (size_t i = 0; i != maos.size(); ++i) {
    std::cout << "maos[" << i << "] = ( x:" << maos[i].x << ", y:" << maos[i].y << ", z:" << maos[i].z
              << ", value:" << maos[i].value << ")\n";
  }
}