// SoA structure with an AoS interface using P2996 reflection and P3294 token
// injection. Each array in the SoA is allocated in a contiguous storage
// container.

#define __cpp_lib_reflection 20250130 // eccp version

#include "rmpp.h"
#include <cassert>
#include <cmath>
#include <experimental/meta>
#include <iostream>
#include <tuple>

using namespace std::literals::string_view_literals;

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
//   std::array<std::array<float, 2> , 3>  matrix;

  void SetId(int id) { m_particleID = id; }

  double pt2() const { return m_momentum.Pt2(); }
};

using SoA = rmpp::AoS2SoA<Particle, 64>;

int main() {
  constexpr size_t n = 3;
  alignas(64) std::vector<std::byte> buf(SoA::ComputeSize(n));
  SoA maos(buf.data(), buf.size(), n);

  maos.push_back({0, {0,0,0,0}, {33,33,33}, {1, 2}});
  maos.push_back({1, {1,1,1,1}, {44,44,44}, {3, 4, 5, 6}});
  maos.push_back({2, {2,2,2,2}, {55,55,55}, {7}});

  std::cout << "----------- Before ------------\n";
  print_soa_as_aos(maos);

  maos[0].SetId(9);
  maos[1].m_referencePoint.SetX(9999);
  maos[1].m_referencePoint.fCoordinates.fZ = 8888;
  maos[2].m_momentum.SetPxPyPzE(0, 0, 0, 0);

  std::cout << "------------ After -----------\n";
  print_soa_as_aos(maos);
  std::cout << "maos[2].pt2() = " << maos[2].pt2() << "\n";

  return 0;
}
