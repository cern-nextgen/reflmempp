#include "utils.h"

#include <array>
#include <iostream>
#include <span>
#include <vector>

using namespace std::literals::string_view_literals;

constexpr size_t mDim = 2;

namespace mds {

template <typename T, size_t Alignment>
class vector {
public:
  std::vector<std::byte> storage;

  struct sov_metadata {
    size_t offset, size;
    friend std::ostream &operator<<(std::ostream &os, const sov_metadata &obj) {
      return os << "{" << obj.offset << ", " << obj.size << "}";
    }
  };

  size_t _size; // Number of elements

  // SoV
  std::span<double> _x;
  std::span<int> _v;
  std::span<float> _m;

  std::vector<size_t> byte_sizes; // Size of each SoV including alignment padding
  std::vector<sov_metadata> _v_md;

  struct aos_view {
    using m_extents = extents<size_t, mDim, mDim>;

    const double &x;
    const std::span<int> v;
    const mdspan<float, m_extents, layout_stride> m;
  };

  // Helper function to compute aligned size
  constexpr inline size_t align_size(size_t size, size_t alignment) const {
    return ((size + alignment - 1) / alignment) * alignment;
  }

public:
  vector(std::initializer_list<T> data) {
    constexpr size_t n_members = 4;
    _size = data.size();

    byte_sizes.resize(n_members);
    std::vector<size_t> sizes(n_members);
    size_t total_byte_size = 0;

    // Compute SoV sizes
    size_t m_idx = 0;
    byte_sizes[m_idx] = align_size(sizeof(double[_size]), Alignment); // _x
    sizes[m_idx] = _size;
    std::cout << "_x = " << sizes[m_idx] << " elements in " << byte_sizes[m_idx] << " bytes\n";
    total_byte_size += byte_sizes[m_idx++];

    for (auto &elem : data) { // _v
      auto n_elements = elem.v.size();
      _v_md.push_back({.offset = sizes[m_idx], .size = n_elements});
      byte_sizes[m_idx] += align_size(sizeof(int[n_elements]), Alignment);
      sizes[m_idx] += n_elements;
    }
    std::cout << "_v = " << sizes[m_idx] << " elements in " << byte_sizes[m_idx] << " bytes\n";
    total_byte_size += byte_sizes[m_idx++];

    byte_sizes[m_idx] = align_size(sizeof(float[_size * mDim * mDim]), Alignment); // _x
    sizes[m_idx] = _size * mDim * mDim;
    std::cout << "_m = " << sizes[m_idx] << " elements in " << byte_sizes[m_idx] << " bytes\n";
    total_byte_size += byte_sizes[m_idx++];

    storage.resize(total_byte_size);
    std::cout << "storage of " << total_byte_size << " bytes in total\n\n";

    // Loop over storage vectors
    size_t offset = 0;
    m_idx = 0;

    _x = std::span(reinterpret_cast<double *>(storage.data() + offset), sizes[m_idx]);
    offset += byte_sizes[m_idx++];
    size_t e_idx = 0;
    for (auto &elem : data) {
      std::construct_at(&_x[e_idx], elem.x);
      e_idx++;
    }

    _v = std::span(reinterpret_cast<int *>(storage.data() + offset), sizes[m_idx]);
    offset += byte_sizes[m_idx++];
    e_idx = 0;
    for (auto &elem : data) {
      for (size_t i = 0; i < elem.v.size(); i++) {
        std::construct_at(&_v[e_idx], elem.v[i]);
        e_idx++;
      }
    }

    _m = std::span(reinterpret_cast<float *>(storage.data() + offset), sizes[m_idx]);
    offset += byte_sizes[m_idx++];
    e_idx = 0;
    for (size_t i = 0; i < mDim; i++) {
      for (size_t j = 0; j < mDim; j++) {
        for (auto &elem : data) {
          std::construct_at(&_m[e_idx], elem.m[i][j]);
          e_idx++;
        }
      }
    }
  }

  size_t size() const { return _size; }

  aos_view operator[](std::size_t idx) const  {
    auto m_stride = std::array<size_t, 2>{_size * mDim, _size};

    return aos_view{.x = _x[idx],
                    .v = _v.subspan(_v_md[idx].offset, _v_md[idx].size),
                    .m = {_m.data() + idx, {typename aos_view::m_extents{}, m_stride}}};
  }
};
} // namespace mds

struct data {
  double x;
  std::vector<int> v;
  EigenMatrix<float, mDim> m;
};

int main() {
  data e1 = {0, {10, 11, 12, 13}, {{{100, 101}, {102, 103}}}};
  data e2 = {4, {20}, {{{200, 201}, {202, 203}}}};
  data e3 = {8, {30, 31}, {{{300, 301}, {302, 303}}}};

  mds::vector<data, 64> maos = {e1, e2, e3};

  std::cout << "maos.size = " << maos.size() << "\n";
  for (size_t i = 0; i != maos.size(); ++i) {
    std::cout << "maos[" << i << "] = ( x:" << maos[i].x << ", v: {";
    for (size_t j = 0; j < maos[i].v.size(); j++) {
      if (j != 0) std::cout << ", ";
      std::cout << maos[i].v[j];
    }
    std::cout << "}, m: {";
    auto md = maos[i].m;
    for (size_t j = 0; j < md.extent(0); j++) {
      if (j != 0) std::cout << ", ";
      std::cout << "{";
      for (size_t k = 0; k < md.extent(1); k++) {
        if (k != 0) std::cout << ", ";
        std::cout << md[j, k];
      }
      std::cout << "}";
    }
    std::cout << "} )\n";
  }

  for (size_t i = 0; i != maos.size(); ++i) {
    std::cout << "maos[" << i << "] = ( x:" << (long long)&maos[i].x << ", v: {";
    for (size_t j = 0; j < maos[i].v.size(); j++) {
      if (j != 0) std::cout << ", ";
      std::cout << &maos[i].v[j];
    }
    std::cout << "}, m: {";
    auto md = maos[i].m;
    for (size_t j = 0; j < md.extent(0); j++) {
      if (j != 0) std::cout << ", ";
      std::cout << "{";
      for (size_t k = 0; k < md.extent(1); k++) {
        if (k != 0) std::cout << ", ";
        std::cout <<  (long long) &md[j, k];
      }
      std::cout << "}";
    }
    std::cout << "} )\n";
  }

  for (size_t i = 0; i < maos.size(); i++) { std::cout << maos._x[i] << " "; }
  std::cout << "\n";
  for (size_t i = 0; i < maos.size(); i++) { std::cout << maos._v[i] << " "; }
  std::cout << "\n";
  for (size_t i = 0; i < maos.size(); i++) { std::cout << maos._m[i] << " "; }

  std::cout << "\n";
}