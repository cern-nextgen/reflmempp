/**
 * This example is adapted from Oliver Rietmann's SoA wrapper approach:
 *     https://github.com/cern-nextgen/wp1.7-soa-wrapper/commit/21d5b849ef7336bd56ebd896c9f34339a5aa75e3
 * This version reduces code duplication by using C++26 reflection features.
 */

#include "wrapper.h"

#include <algorithm>
#include <assert.h>
#include <iostream>
#include <span>
#include <vector>
#include <print>

#define EXPECT_TRUE(arg)                                                       \
  do {                                                                         \
    if (!(arg)) {                                                              \
      std::cout << "Unexpected false at " << __FILE__ << ", " << __LINE__      \
                << ", " << __func__ << ": " << #arg << std::endl;              \
    }                                                                          \
  } while (false);

struct Inner {
  unsigned int &a;
  unsigned long &b;
};

struct Point {
  float &x;
  int &y;
  double &z;
  MemLayout::RecurseWrap<Inner> inner;

  double sum() const { return x + y + z; };
};
// xxxxyyyyzzzzaaaabbbb
using PointSoA = MemLayout::Wrapper<Point, std::vector>;

using PointVal = MemLayout::Wrapper<Point, MemLayout::value>;
using PointSpan = MemLayout::Wrapper<Point, std::span>;
using PointSoP = MemLayout::Wrapper<Point, MemLayout::pointer>;

int main() {
  PointVal p = {0.0f, 1, 2.0};
  EXPECT_TRUE(p.x == 0.0f && p.y == 1 && p.z == 2.0);

  // {{0.0f, 1.0f, 2.0f}, {5, 4, 3}, {12.0, 21.0, 2.0}, {{10, 11, 12}, {20.0, 21.0, 22.0}}}; doesn't work
  PointSoA soa = {{0.0f, 1.0f, 2.0f}, {5, 4, 3}, {12.0, 21.0, 2.0}};
  soa.inner.a = {10, 11, 12};
  soa.inner.b = {20l, 21l, 22l};

  EXPECT_TRUE(soa[0].x == 0.0f && soa[0].y == 5 && soa[0].z == 12.0);
  EXPECT_TRUE(soa[1].x == 1.0f && soa[1].y == 4 && soa[1].z == 21.0);
  EXPECT_TRUE(soa[2].x == 2.0f && soa[2].y == 3 && soa[2].z == 2.0);
  EXPECT_TRUE(soa[2].sum() == 2.0f + 3 + 2.0);
  for (size_t i : std::views::iota(0u, 3u)) {
    EXPECT_TRUE(soa[i].inner.a == i + 10 && soa[i].inner.b == i + 20.0);
  }

  PointSpan span = soa;
  span[1].x = 213445;
  EXPECT_TRUE(span[1].x == 213445 && span[1].x == soa[1].x);

  PointSoP sop = {soa.x.data(), soa.y.data(), soa.z.data()};
  sop.inner = {soa.inner.a.data(), soa.inner.b.data()};
  EXPECT_TRUE(sop[0].x == 0.0f && sop[0].y == 5 && sop[0].z == 12.0);
  EXPECT_TRUE(sop[1].x == 213445 && sop[1].y == 4 && sop[1].z == 21.0);
  EXPECT_TRUE(sop[2].x == 2.0f && sop[2].y == 3 && sop[2].z == 2.0);

  MemLayout::Wrapper<Point, MemLayout::reference> ref = soa[2];
  EXPECT_TRUE(ref.sum() == 2.0f + 3 + 2.0);

  // auto test = *sop;
  // auto end = sop + 3;
  // auto s = test < end;
  // std::sort(sop, sop + 3);
  // EXPECT_TRUE(sop[0].x == 0.0f && sop[0].y == 5 && sop[0].z == 12.0);
  // EXPECT_TRUE(sop[1].x == 2.0f && sop[1].y == 3 && sop[1].z == 2.0);
  // EXPECT_TRUE(sop[2].x == 213445 && sop[2].y == 4 && sop[2].z == 21.0);

  return 0;
}