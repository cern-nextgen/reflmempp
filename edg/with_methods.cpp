// SoA structure with an AoS interface using P2996 reflection and P3294 token
// injection. Each array in the SoA is allocated in a contiguous storage
// container.

#define __cpp_lib_reflection 20240815

#include "rmpp_ref.h"
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

  void SetY(T yy) { fCoordinates.SetY(yy); }
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
  PositionVector3D<double> m_referencePoint;
  LorentzVector<double> m_momentum;

  void SetId(int id) { m_particleID = id; }

  double pt2() const { return m_momentum.Pt2(); }
};

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

  std::cout << "----------- Before ------------\n";
  print_soa_as_aos(maos);

  maos[0].SetId(9);
  assert(maos.m_particleID[0] == 9);

  maos[1].m_referencePoint.SetY(9999);
  assert(maos.m_referencePoint.fCoordinates.fY[1] == 9999);

  maos[1].m_referencePoint.fCoordinates.fZ = 8888;
  assert(maos.m_referencePoint.fCoordinates.fZ[1] == 8888);

  maos[2].m_momentum.SetPxPyPzE(0, 0, 0, 0);
  assert(maos.m_momentum.fX[2] == 0);
  assert(maos.m_momentum.fY[2] == 0);
  assert(maos.m_momentum.fZ[2] == 0);
  assert(maos.m_momentum.fT[2] == 0);

  std::cout << "------------ After -----------\n";
  print_soa_as_aos(maos);
  std::cout << "maos[2].pt2() = " << maos[2].pt2() << "\n";

  return 0;
}
