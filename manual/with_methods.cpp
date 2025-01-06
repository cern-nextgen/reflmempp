#include "utils.h"

#include <array>
#include <iostream>
#include <span>
#include <vector>

using namespace std::literals::string_view_literals;

constexpr size_t mDim = 2;

struct Vec {
  float &x, &y, &z;
  void MakePlanar() { z = 0; }

  friend std::ostream &operator<<(std::ostream &os, const Vec &obj) {
    return os << "{" << obj.x << ", " << obj.y << ", " << obj.z << "}";
  }
};

struct Particle {
  int &id;
  Vec pos;
  void SetIdMember(int v) { id = v; }
};

namespace rmpp {
template <typename T, size_t Alignment>
class vector {
public:
  std::vector<std::byte> storage;

  struct sopos_metadata {
    size_t offset, size;
    friend std::ostream &operator<<(std::ostream &os, const sopos_metadata &obj) {
      return os << "{" << obj.offset << ", " << obj.size << "}";
    }
  };

  size_t m_size; // Number of elements

  using m_id_type = typeof(Particle::id);
  using m_pos_type = typeof(Particle::pos.x);

  // SoA members
  std::span<m_id_type> id;
  struct VecSoA {
    std::span<m_pos_type> x, y, z;
  };
  VecSoA pos;

  std::vector<size_t> byte_sizes;       // Size of each SoA member including alignment padding
  std::vector<sopos_metadata> m_pos_md; // Offset and size of each (jagged) vector in m_v;

  using aos_view = T;
  struct aos_cview {
    const m_id_type &id;
    const Vec pos;
  };

  struct aos_type {
    struct Vec {
      m_pos_type x, y, z;
    };
    m_id_type id;
    Vec pos;
  };

  struct soa_type {
    std::initializer_list<m_id_type> id;
    std::initializer_list<m_pos_type> pos_x, pos_y, pos_z;
  };

  // Helper function to compute aligned size
  constexpr inline size_t align_size(size_t size, size_t alignment) const {
    return ((size + alignment - 1) / alignment) * alignment;
  }

public:
  vector(size_t n) {
    m_size = n;
    constexpr size_t vec_n_members = 3;

    size_t total_byte_size = n * sizeof(m_id_type) + n * sizeof(m_pos_type) * vec_n_members;
    storage.resize(total_byte_size);

    id = std::span(reinterpret_cast<m_id_type *>(storage.data()), n);
    pos.x = std::span(reinterpret_cast<m_pos_type *>(storage.data() + n * sizeof(m_id_type)), n);
    pos.y =
        std::span(reinterpret_cast<m_pos_type *>(storage.data() + n * sizeof(m_id_type) + sizeof(m_pos_type) * n), n);
    pos.z = std::span(
        reinterpret_cast<m_pos_type *>(storage.data() + n * sizeof(m_id_type) + 2 * sizeof(m_pos_type) * n), n);

  }

  size_t size() const { return m_size; }

  aos_view operator[](std::size_t idx) {
    return aos_view{.id = id[idx], .pos = {pos.x[idx], pos.y[idx], pos.z[idx]}};
  }

  aos_cview operator[](std::size_t idx) const {
    return aos_cview{.id = id[idx], .pos = {pos.x[idx], pos.y[idx], pos.z[idx]}};
  }
};
} // namespace rmpp

using SoA = rmpp::vector<Particle, 64>;

int main() {
  SoA maos(3);
  std::array<int, 3> ids = {0, 1, 2};
  std::array<float, 3> pos_x = {1, 4, 7};
  std::array<float, 3> pos_y = {2, 5, 8};
  std::array<float, 3> pos_z = {3, 6, 9};
  std::copy(ids.begin(), ids.end(), maos.id.begin());
  std::copy(pos_x.begin(), pos_x.end(), maos.pos.x.begin());
  std::copy(pos_y.begin(), pos_y.end(), maos.pos.y.begin());
  std::copy(pos_z.begin(), pos_z.end(), maos.pos.z.begin());

  std::cout << "---- Before ---\nmaos.size = " << maos.size() << "\n";
  for (size_t i = 0; i != maos.size(); ++i) {
    std::cout << "maos[" << i << "] = ( Id:" << maos[i].id;
    std::cout << "}, Vec: {" << maos[i].pos;
    std::cout << "}\n";
  }

  maos[0].SetIdMember(42);
  maos[0].pos.MakePlanar();

  std::cout << "---- After ----\n";
  for (size_t i = 0; i != maos.size(); ++i) {
    std::cout << "maos[" << i << "] = ( Id:" << maos[i].id;
    std::cout << "}, Vec: {" << maos[i].pos;
    std::cout << "}\n";
  }
}