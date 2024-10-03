#include <iostream>
#include <span>
#include <vector>

// dummy
struct data {
  double x, y, z, value;
};

namespace mds {

template <typename T, size_t Alignment> class vector {
private:
  std::vector<std::byte> storage;

  std::span<double> _x, _y, _z, _value;
  std::vector<size_t> sizes; // Number of data elements per data vector
  struct aos_view {
    const double &x, &y, &z, &value;
  };

  // Helper function to compute aligned size
  constexpr inline size_t align_size(size_t size, size_t alignment) const {
    return ((size + alignment - 1) / alignment) * alignment;
  }

public:
  vector(std::initializer_list<T> data) {
    size_t n_members = 4;

    size_t total_size = 0;
    std::vector<size_t> byte_sizes;
    byte_sizes.reserve(n_members);
    sizes.reserve(n_members);

    for (size_t m_idx = 0; m_idx < n_members; m_idx++) {
      byte_sizes.push_back(align_size(data.size() * sizeof(double), Alignment));
      sizes.push_back(data.size());
      total_size += byte_sizes[m_idx];
    }

    storage.resize(total_size);
    std::cout << "storage of " << total_size << " bytes in total\n\n";

    // Loop over storage vectors
    size_t offset = 0;
    size_t m_idx = 0;

    _x = std::span(reinterpret_cast<double *>(storage.data() + offset),
                   byte_sizes[m_idx] / sizeof(double));
    offset += byte_sizes[m_idx++];
    size_t e_idx = 0;
    for (auto elem : data) {
      new (&_x[e_idx]) double(elem.x);
      e_idx++;
    }

    _y = std::span(reinterpret_cast<double *>(storage.data() + offset),
                   byte_sizes[m_idx] / sizeof(double));
    offset += byte_sizes[m_idx];
    e_idx = 0;
    for (auto elem : data) {
      new (&_y[e_idx]) double(elem.y);
      e_idx++;
    }

    _z = std::span(reinterpret_cast<double *>(storage.data() + offset),
                   byte_sizes[m_idx] / sizeof(double));
    offset += byte_sizes[m_idx];
    e_idx = 0;
    for (auto elem : data) {
      new (&_z[e_idx]) double(elem.z);
      e_idx++;
    }

    _value = std::span(reinterpret_cast<double *>(storage.data() + offset),
                       byte_sizes[m_idx] / sizeof(double));
    offset += byte_sizes[m_idx];
    e_idx = 0;
    for (auto elem : data) {
      new (&_value[e_idx]) double(elem.value);
      e_idx++;
    }
  }

  size_t size() const { return sizes[0]; }

  aos_view operator[](std::size_t idx) const  {
    return aos_view{_x[idx], _y[idx], _z[idx], _value[idx]};
  }
};
} // namespace mds

int main() {
  data e1 = {0, 1, 2, 3};
  data e2 = {4, 5, 6, 7};
  data e3 = {8, 9, 10, 11};

  mds::vector<data, 64> maos = {e1, e2, e3};

  std::cout << "maos.size = " << maos.size() << "\n";
  for (size_t i = 0; i != maos.size(); ++i) {
    std::cout << "maos[" << i << "] = ({ x:" << maos[i].x << ", y:" << maos[i].y
              << ", z:" << maos[i].z << ", value:" << maos[i].value;
    std::cout << "})\n";
  }
}