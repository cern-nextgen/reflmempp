#include "utils.h"
#include <iostream>
#include <vector>

namespace rmpp {

static size_t max_inner_array_size = 8; // max objents for inner array per data objent

template <typename T>
struct ValHelper {
  struct SVal;
};

template <typename S, size_t Alignment>
class AoS2SoA {
private: //
  std::span<std::byte> storage;
  size_t m_size, m_capacity;

  constexpr static inline size_t align_size(size_t size) { return ((size + Alignment - 1) / Alignment) * Alignment; }

public:
  /// SoA Members
  std ::span<int> m_particleID;
  struct LorentzVectorSoA {
    std ::span<double> fX;
    std ::span<double> fY;
    std ::span<double> fZ;
    std ::span<double> fT;
  };
  LorentzVectorSoA m_momentum;
  struct PositionVector3DSoA {
    struct Cartesian3DSoA {
      std ::span<double> fX;
      std ::span<double> fY;
      std ::span<double> fZ;
    };
    Cartesian3DSoA fCoordinates;
  };
  PositionVector3DSoA m_referencePoint;
  std ::span<inner_type<std::span<float>>> vector;
  std ::vector<size_t> vector_offsets;

  // View Types
  using aos_view = S;
  using aos_cview = const S;

  using value_type = ValHelper<S>::SVal;

  static size_t ComputeSize(size_t n) {
    return align_size(0 + n * sizeof(int) +                      // particleID
                      n * align_size(sizeof(double) * 4) +       // LorentzVector
                      n * align_size(sizeof(double) * 3) +       // PositionVector3D
                      n * align_size(sizeof(double) * 3) +       // Cartesian3D
                      n * max_inner_array_size * sizeof(float)); // vector
  }

  AoS2SoA(std::byte *buf, const size_t buf_size, const size_t capacity) {
    storage = std::span<std::byte>(buf, buf_size);
    m_size = 0;
    m_capacity = capacity;

    // Initialize the spans for each member in the SoA to offsets into
    // the storage container.
    m_particleID = std ::span(reinterpret_cast<int *>(storage.data() + 0), m_capacity);
    m_momentum.fX =
        std ::span(reinterpret_cast<double *>(storage.data() + 0 + align_size(m_particleID.size_bytes())), m_capacity);
    m_momentum.fY = std ::span(reinterpret_cast<double *>(storage.data() + 0 + align_size(m_particleID.size_bytes()) +
                                                          align_size(m_momentum.fX.size_bytes())),
                               m_capacity);
    m_momentum.fZ = std ::span(reinterpret_cast<double *>(storage.data() + 0 + align_size(m_particleID.size_bytes()) +
                                                          align_size(m_momentum.fX.size_bytes()) +
                                                          align_size(m_momentum.fY.size_bytes())),
                               m_capacity);
    m_momentum.fT = std ::span(reinterpret_cast<double *>(storage.data() + 0 + align_size(m_particleID.size_bytes()) +
                                                          align_size(m_momentum.fX.size_bytes()) +
                                                          align_size(m_momentum.fY.size_bytes()) +
                                                          align_size(m_momentum.fZ.size_bytes())),
                               m_capacity);
    m_referencePoint.fCoordinates.fX = std ::span(
        reinterpret_cast<double *>(storage.data() + 0 + align_size(m_particleID.size_bytes()) +
                                   align_size(m_momentum.fX.size_bytes()) + align_size(m_momentum.fY.size_bytes()) +
                                   align_size(m_momentum.fZ.size_bytes()) + align_size(m_momentum.fT.size_bytes())),
        m_capacity);
    m_referencePoint.fCoordinates.fY = std ::span(
        reinterpret_cast<double *>(storage.data() + 0 + align_size(m_particleID.size_bytes()) +
                                   align_size(m_momentum.fX.size_bytes()) + align_size(m_momentum.fY.size_bytes()) +
                                   align_size(m_momentum.fZ.size_bytes()) + align_size(m_momentum.fT.size_bytes()) +
                                   align_size(m_referencePoint.fCoordinates.fX.size_bytes())),
        m_capacity);
    m_referencePoint.fCoordinates.fZ = std ::span(
        reinterpret_cast<double *>(storage.data() + 0 + align_size(m_particleID.size_bytes()) +
                                   align_size(m_momentum.fX.size_bytes()) + align_size(m_momentum.fY.size_bytes()) +
                                   align_size(m_momentum.fZ.size_bytes()) + align_size(m_momentum.fT.size_bytes()) +
                                   align_size(m_referencePoint.fCoordinates.fX.size_bytes()) +
                                   align_size(m_referencePoint.fCoordinates.fY.size_bytes())),
        m_capacity);
    vector = std ::span(reinterpret_cast<inner_type<std::span<float>> *>(
                            storage.data() + 0 + align_size(m_particleID.size_bytes()) +
                            align_size(m_momentum.fX.size_bytes()) + align_size(m_momentum.fY.size_bytes()) +
                            align_size(m_momentum.fZ.size_bytes()) + align_size(m_momentum.fT.size_bytes()) +
                            align_size(m_referencePoint.fCoordinates.fX.size_bytes()) +
                            align_size(m_referencePoint.fCoordinates.fY.size_bytes()) +
                            align_size(m_referencePoint.fCoordinates.fZ.size_bytes())),
                        m_capacity * max_inner_array_size);
    vector_offsets.resize(m_capacity + 1);
  }

  size_t size() const { return m_size; }

  aos_view operator[](const size_t idx) {
    return aos_view{m_particleID[idx],
                    {m_momentum.fX[idx], m_momentum.fY[idx], m_momentum.fZ[idx], m_momentum.fT[idx]},
                    {m_referencePoint.fCoordinates.fX[idx], m_referencePoint.fCoordinates.fY[idx],
                     m_referencePoint.fCoordinates.fZ[idx]},
                    vector.subspan(vector_offsets[idx], vector_offsets[idx + 1] - vector_offsets[idx])};
  }

  aos_cview operator[](const size_t idx) const {
    return aos_cview{m_particleID[idx],
                     {m_momentum.fX[idx], m_momentum.fY[idx], m_momentum.fZ[idx], m_momentum.fT[idx]},
                     {m_referencePoint.fCoordinates.fX[idx], m_referencePoint.fCoordinates.fY[idx],
                      m_referencePoint.fCoordinates.fZ[idx]},
                     vector.subspan(vector_offsets[idx], vector_offsets[idx + 1] - vector_offsets[idx])};
  }

  void push_back(const value_type obj) & {
    if (m_size == m_capacity) {
      std::cerr << "Capacity reached" << std::endl;
      return;
    }

    new (&m_particleID[m_size]) int(obj.m_particleID);
    new (&m_momentum.fX[m_size]) double(obj.m_momentum.fX);
    new (&m_momentum.fY[m_size]) double(obj.m_momentum.fY);
    new (&m_momentum.fZ[m_size]) double(obj.m_momentum.fZ);
    new (&m_momentum.fT[m_size]) double(obj.m_momentum.fT);
    new (&m_referencePoint.fCoordinates.fX[m_size]) double(obj.m_referencePoint.fCoordinates.fX);
    new (&m_referencePoint.fCoordinates.fY[m_size]) double(obj.m_referencePoint.fCoordinates.fY);
    new (&m_referencePoint.fCoordinates.fZ[m_size]) double(obj.m_referencePoint.fCoordinates.fZ);
    for (size_t i = 0; i < obj.vector.size(); ++i) {
      new (&vector[vector_offsets[m_size] + i]) float(obj.vector[i]);
    }
    vector_offsets[m_size + 1] = vector_offsets[m_size] + obj.vector.size();

    m_size++;
  }
};
} // namespace rmpp

///////////////////
/// EXAMPLE
///////////////////
template <typename T>
struct Cartesian3D {
  T &fX, &fY, &fZ;

  void SetY(T yy) { fY = yy; }
};

template <typename T>
struct PositionVector3D {
  Cartesian3D<T> fCoordinates;

  void SetX(T yy) { fCoordinates.SetY(yy); }
};

template <typename T>
struct LorentzVector {
  T &fX, &fY, &fZ, &fT;

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
  LorentzVector<double> m_momentum;
  PositionVector3D<double> m_referencePoint;
  std::span<float> vector;

  void SetId(int id) { m_particleID = id; }
  double pt2() const { return m_momentum.Pt2(); }
};

namespace rmpp {
template <>
struct ValHelper<Particle> {
  struct SVal {
    int m_particleID;
    struct LorentzVector {
      double fX, fY, fZ, fT;
    } m_momentum;
    struct PositionVector3D {
      struct Cartesian3D {
        double fX, fY, fZ;
      } fCoordinates;
    } m_referencePoint;
    std::vector<float> vector;
  };
};
} // namespace rmpp

using SoA = rmpp::AoS2SoA<Particle, 64>;

int main() {
  constexpr size_t n = 3;
  alignas(64) std::vector<std::byte> buf(SoA::ComputeSize(n));
  SoA maos(buf.data(), buf.size(), n);

  maos.push_back({0, {0, 0, 0, 0}, {33, 33, 33}, {1, 2}});
  maos.push_back({1, {1, 1, 1, 1}, {44, 44, 44}, {3, 4, 5, 6}});
  maos.push_back({2, {2, 2, 2, 2}, {55, 55, 55}, {7}});

  std::cout << "----------- Before ------------\n";
  for (size_t i = 0; i < maos.size(); ++i) {
    std::cout << "maos[" << i << "] = (\n";
    std::cout << "\tm_particleID: " << maos[i].m_particleID << "\n";
    std::cout << "\tm_momentum: (" << maos[i].m_momentum.fX << ", " << maos[i].m_momentum.fY << ", "
              << maos[i].m_momentum.fZ << ", " << maos[i].m_momentum.fT << ")\n";
    std::cout << "\tm_referencePoint: (" << maos[i].m_referencePoint.fCoordinates.fX << ", "
              << maos[i].m_referencePoint.fCoordinates.fY << ", " << maos[i].m_referencePoint.fCoordinates.fZ << ")\n";
    std::cout << "\tvector: (";
    for (const auto &v : maos[i].vector) {
      std::cout << v << " ";
    }
    std::cout << ")\n";
  }

  maos[0].SetId(9);
  maos[1].m_referencePoint.SetX(9999);
  maos[1].m_referencePoint.fCoordinates.fZ = 8888;
  maos[2].m_momentum.SetPxPyPzE(0, 0, 0, 0);

  std::cout << "------------ After -----------\n";
  for (size_t i = 0; i < maos.size(); ++i) {
    std::cout << "maos[" << i << "] = (\n";
    std::cout << "\tm_particleID: " << maos[i].m_particleID << "\n";
    std::cout << "\tm_momentum: (" << maos[i].m_momentum.fX << ", " << maos[i].m_momentum.fY << ", "
              << maos[i].m_momentum.fZ << ", " << maos[i].m_momentum.fT << ")\n";
    std::cout << "\tm_referencePoint: (" << maos[i].m_referencePoint.fCoordinates.fX << ", "
              << maos[i].m_referencePoint.fCoordinates.fY << ", " << maos[i].m_referencePoint.fCoordinates.fZ << ")\n";
    std::cout << "\tvector: ( ";
    for (const auto &v : maos[i].vector) {
      std::cout << v << " ";
    }
    std::cout << ")\n";
  }
  std::cout << "maos[2].pt2() = " << maos[2].pt2() << "\n";

  return 0;
}
