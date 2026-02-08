/**
 * This example is adapted from Oliver Rietmann's SoA wrapper approach:
 *     https://github.com/cern-nextgen/wp1.7-soa-wrapper/commit/21d5b849ef7336bd56ebd896c9f34339a5aa75e3
 * This version reduces code duplication by using C++26 reflection features.
 */

#include "wrapper.h"

#include <iostream>
#include <span>
#include <vector>

///////////////////

struct Point {
  float &x;
  int &y;
  double &z;

  double sum() const { return x + y + z; };
};

using PointSoA = Wrapper<Point, std::vector>;
using PointVal = Wrapper<Point, value_type>;
using PointSpan = Wrapper<Point, std::span>;

int main() {
  PointVal p = {0.0f, 1, 2.0};
  PointSoA q = {{0.0f, 0.0f, 0.0f}, {1, 1, 1}, {2.0, 2.0, 2.0}};

  PointSpan s = q;
  s[1].x = 213445;

  q[0].z = 42;
  std::cout << "q[0] = { x: " << q[0].x << ", y: " << q[0].y << ", z: " << q[0].z << " }" << std::endl;
  std::cout << "q[1] = { x: " << q[1].x << ", y: " << q[1].y << ", z: " << q[1].z << " }" << std::endl;
  std::cout << "q[2] = { x: " << q[2].x << ", y: " << q[2].y << ", z: " << q[2].z << " }" << std::endl;
  std::cout << "sum q[2] == " << q[2].sum() << std::endl;

  std::cout << "q.y[1] = " << q.y[1] << std::endl;
  return 0;
}