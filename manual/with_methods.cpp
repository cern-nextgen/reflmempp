#include "utils.h"

#include <array>
#include <iostream>
#include <span>
#include <vector>

using namespace std::literals::string_view_literals;

constexpr size_t mDim = 2;

template <typename T>
struct Cartesian3D {
  T &fX, &fY, &fZ;

  friend std::ostream &operator<<(std::ostream &os, const Cartesian3D<T> &obj) {
    return os << "{" << obj.fX << ", " << obj.fY << ", " << obj.fZ << "}";
  }

  void print_addr() const {
    std::cout << "{" << (long long)&fX << ", " << (long long)&fY << ", " << (long long)&fZ << "}";
  }

  void SetY(T yy) { fY = yy; }
};

template <typename T>
struct PositionVector3D {
  Cartesian3D<T> fCoordinates;

  friend std::ostream &operator<<(std::ostream &os, const PositionVector3D<T> &obj) {
    return os << "{" << obj.fCoordinates << "}";
  }

  void print_addr() const { fCoordinates.print_addr(); }

  void SetX(T yy) { fCoordinates.SetY(yy); }
};

template <typename T>
struct LorentzVector {
  T &fX, &fY, &fZ, &fT;

  friend std::ostream &operator<<(std::ostream &os, const LorentzVector<T> &obj) {
    return os << "{" << obj.fX << ", " << obj.fY << ", " << obj.fZ << ", " << obj.fT << "}";
  }

  void print_addr() const {
    std::cout << "{" << (long long)&fX << ", " << (long long)&fY << ", " << (long long)&fZ << ", " << (long long)&fT
              << "}";
  }

  // std::sqrt currently doesn't work with the experimental EDG reflection compiler?
  T Pt2() const { return fX * fX + fY * fY; }
  void SetPxPyPzE(T px, T py, T pz, T e) {
    fX = px;
    fY = py;
    fZ = pz;
    fT = e;
  }
};

struct Particle {
  int &m_particleID;
  PositionVector3D<double> m_referencePoint;
  LorentzVector<double> m_momentum;

  void SetId(int id) { m_particleID = id; }

  double pt2() const { return m_momentum.Pt2(); }
};

//////////////////////////

namespace rmpp {
template <typename T, size_t Alignment>
class vector {
public:
  std::vector<std::byte> storage;

  size_t m_size; // Number of elements

  // Start SoA members
  std::span<int> m_particleID;

  struct PositionVector3DSoA {
    struct Cartesian3DSoA {
      std::span<double> fX, fY, fZ;
    };
    Cartesian3DSoA fCoordinates;
  };
  PositionVector3DSoA m_referencePoint;

  struct LorentzVectorSoA {
    std::span<double> fX, fY, fZ, fT;
  };
  LorentzVectorSoA m_momentum;
  // End SoA members

  using aos_view = T;

  // Helper function to compute aligned size
  constexpr inline size_t align_size(size_t size) const { return ((size + Alignment - 1) / Alignment) * Alignment; }

public:
  vector(size_t n) {
    m_size = n;
    size_t total_byte_size = 0;

    total_byte_size += align_size(sizeof(int[m_size]));        // m_particleID
    total_byte_size += align_size(sizeof(double[m_size]) * 3); // m_referencePoint
    total_byte_size += align_size(sizeof(double[m_size]) * 4); // m_momentum
    storage.resize(total_byte_size);

    size_t offset = 0;
    m_particleID = std::span(reinterpret_cast<int *>(storage.data() + offset), m_size);
    new (storage.data() + offset) int[m_size];
    offset += sizeof(int[m_size]);

    m_referencePoint.fCoordinates.fX = std::span(reinterpret_cast<double *>(storage.data() + offset), m_size);
    new (storage.data() + offset) double[m_size];
    offset += sizeof(double[m_size]);

    m_referencePoint.fCoordinates.fY = std::span(reinterpret_cast<double *>(storage.data() + offset), m_size);
    new (storage.data() + offset) double[m_size];
    offset += sizeof(double[m_size]);

    m_referencePoint.fCoordinates.fZ = std::span(reinterpret_cast<double *>(storage.data() + offset), m_size);
    new (storage.data() + offset) double[m_size];
    offset += sizeof(double[m_size]);

    m_momentum.fX = std::span(reinterpret_cast<double *>(storage.data() + offset), m_size);
    new (storage.data() + offset) double[m_size];
    offset += sizeof(double[m_size]);

    m_momentum.fY = std::span(reinterpret_cast<double *>(storage.data() + offset), m_size);
    new (storage.data() + offset) double[m_size];
    offset += sizeof(double[m_size]);

    m_momentum.fZ = std::span(reinterpret_cast<double *>(storage.data() + offset), m_size);
    new (storage.data() + offset) double[m_size];
    offset += sizeof(double[m_size]);

    m_momentum.fT = std::span(reinterpret_cast<double *>(storage.data() + offset), m_size);
    new (storage.data() + offset) double[m_size];
  }

  size_t size() const { return m_size; }

  aos_view operator[](std::size_t idx) {
    return aos_view{
        .m_particleID = m_particleID[idx],
        .m_referencePoint = {.fCoordinates = {.fX = m_referencePoint.fCoordinates.fX[idx],
                                              .fY = m_referencePoint.fCoordinates.fY[idx],
                                              .fZ = m_referencePoint.fCoordinates.fZ[idx]}},
        .m_momentum = {.fX = m_momentum.fX[idx],
                       .fY = m_momentum.fY[idx],
                       .fZ = m_momentum.fZ[idx],
                       .fT = m_momentum.fT[idx]},

    };
  }
};
} // namespace rmpp

using SoA = rmpp::vector<Particle, 64>;

int main() {
  SoA maos(3);

  std::array<int, 3> ids = {0, 1, 2};
  std::array<float, 3> ref_x = {3, 6, 9};
  std::array<float, 3> ref_y = {4, 7, 10};
  std::array<float, 3> ref_z = {5, 8, 11};
  std::array<float, 3> momentum_x = {12, 16, 20};
  std::array<float, 3> momentum_y = {13, 17, 21};
  std::array<float, 3> momentum_z = {14, 18, 22};
  std::array<float, 3> momentum_t = {15, 19, 23};

  std::copy(ids.begin(), ids.end(), maos.m_particleID.begin());
  std::copy(ref_x.begin(), ref_x.end(), maos.m_referencePoint.fCoordinates.fX.begin());
  std::copy(ref_y.begin(), ref_y.end(), maos.m_referencePoint.fCoordinates.fY.begin());
  std::copy(ref_z.begin(), ref_z.end(), maos.m_referencePoint.fCoordinates.fZ.begin());
  std::copy(momentum_x.begin(), momentum_x.end(), maos.m_momentum.fX.begin());
  std::copy(momentum_y.begin(), momentum_y.end(), maos.m_momentum.fY.begin());
  std::copy(momentum_z.begin(), momentum_z.end(), maos.m_momentum.fZ.begin());
  std::copy(momentum_t.begin(), momentum_t.end(), maos.m_momentum.fT.begin());

  std::cout << "---- Before ---\nmaos.size = " << maos.size() << "\n";
  for (size_t i = 0; i != maos.size(); ++i) {
    std::cout << "maos[" << i << "] = (\n";
    std::cout << "\tm_particleID: " << maos[i].m_particleID << "\n";
    std::cout << "\tm_referencePoint: " << maos[i].m_referencePoint << "\n";
    std::cout << "\tm_momentum: " << maos[i].m_momentum << "\n";
    std::cout << ")\n";
  }

  maos[0].SetId(9);
  maos[1].m_referencePoint.SetX(9999);
  maos[1].m_referencePoint.fCoordinates.fZ = 8888;
  maos[2].m_momentum.SetPxPyPzE(0, 0, 0, 0);

  std::cout << "\n---- After ----\n";
  for (size_t i = 0; i != maos.size(); ++i) {
    std::cout << "maos[" << i << "] = (\n";
    std::cout << "\tm_particleID: " << maos[i].m_particleID << "\n";
    std::cout << "\tm_referencePoint: " << maos[i].m_referencePoint << "\n";
    std::cout << "\tm_momentum: " << maos[i].m_momentum << "\n";
    std::cout << ")\n";
  }
}