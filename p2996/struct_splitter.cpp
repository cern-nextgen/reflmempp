#include "struct_splitter.h"
#include <gtest/gtest.h>

struct Point {
  float &x;
  int &y;
  double &z;
};

template <size_t... Is>
struct SubPoint;

consteval {
  SplitStruct<Point, SubPoint>(SplitOp({1, 2, 0}), SplitOp({0}), SplitOp({1}), SplitOp({2}), SplitOp({2, 1}));
}
using PointAoS = PartitionedContainer<Point, SubPoint<1, 2, 0>>;
using PointSoA = PartitionedContainer<Point, SubPoint<0>, SubPoint<1>, SubPoint<2>>;
using PointSoAoS = PartitionedContainer<Point, SubPoint<0>, SubPoint<2,1>>;

// Byte address of an object, so that distances between members of different types can be compared.
template <typename T>
static const std::byte *addr(const T &v) {
  return reinterpret_cast<const std::byte *>(&v);
}

TEST(StructSplitter, reordered_aos) {
  PointAoS q(3, 64);

  for (size_t i = 0; i < q.size(); ++i) {
    q[i].x = static_cast<float>(i);
    q[i].y = static_cast<int>(i);
    q[i].z = static_cast<double>(i);
  }

  q[0].z = 42;
  EXPECT_EQ(q[0].x, 0.0f); EXPECT_EQ(q[0].y, 0); EXPECT_EQ(q[0].z, 42.0);
  EXPECT_EQ(q[1].x, 1.0f); EXPECT_EQ(q[1].y, 1); EXPECT_EQ(q[1].z, 1.0);
  EXPECT_EQ(q[2].x, 2.0f); EXPECT_EQ(q[2].y, 2); EXPECT_EQ(q[2].z, 2.0);

  // Expected layout: yzxyzxyzx. All members live in a single array of SubPoint<1, 2, 0>, so
  // within one element the members appear in the order y, z, x, and each member advances by
  // exactly one struct (including padding) from one element to the next.
  constexpr size_t stride = sizeof(SubPoint<1, 2, 0>);
  for (size_t i = 0; i < q.size(); ++i) {
    EXPECT_LT(addr(q[i].y), addr(q[i].z)) << "y not before z at index " << i;
    EXPECT_LT(addr(q[i].z), addr(q[i].x)) << "z not before x at index " << i;
    EXPECT_LE(addr(q[i].x) + sizeof(float), addr(q[0].y) + (i + 1) * stride)
        << "x overflows element " << i;
  }
  for (size_t i = 0; i + 1 < q.size(); ++i) {
    EXPECT_EQ(addr(q[i + 1].x), addr(q[i].x) + stride) << "x stride wrong at index " << i;
    EXPECT_EQ(addr(q[i + 1].y), addr(q[i].y) + stride) << "y stride wrong at index " << i;
    EXPECT_EQ(addr(q[i + 1].z), addr(q[i].z) + stride) << "z stride wrong at index " << i;
  }
  EXPECT_EQ(reinterpret_cast<std::uintptr_t>(&q[0].y) % 64, 0u);
}

TEST(StructSplitter, soa) {
  PointSoA q(3, 64);

  for (size_t i = 0; i < q.size(); ++i) {
    q[i].x = static_cast<float>(i);
    q[i].y = static_cast<int>(i);
    q[i].z = static_cast<double>(i);
  }

  q[0].z = 42;
  EXPECT_EQ(q[0].x, 0.0f); EXPECT_EQ(q[0].y, 0); EXPECT_EQ(q[0].z, 42.0);
  EXPECT_EQ(q[1].x, 1.0f); EXPECT_EQ(q[1].y, 1); EXPECT_EQ(q[1].z, 1.0);
  EXPECT_EQ(q[2].x, 2.0f); EXPECT_EQ(q[2].y, 2); EXPECT_EQ(q[2].z, 2.0);

  // In the SoA layout consecutive elements of the same member must be exactly one element apart.
  for (size_t i = 0; i + 1 < q.size(); ++i) {
    EXPECT_EQ(&q[i + 1].x, &q[i].x + 1) << "x not contiguous at index " << i;
    EXPECT_EQ(&q[i + 1].y, &q[i].y + 1) << "y not contiguous at index " << i;
    EXPECT_EQ(&q[i + 1].z, &q[i].z + 1) << "z not contiguous at index " << i;
  }

  // Each member array is allocated separately with 64-byte alignment (the alignment passed
  // to the constructor), so the start of each array must be aligned accordingly.
  EXPECT_EQ(reinterpret_cast<std::uintptr_t>(&q[0].x) % 64, 0u);
  EXPECT_EQ(reinterpret_cast<std::uintptr_t>(&q[0].y) % 64, 0u);
  EXPECT_EQ(reinterpret_cast<std::uintptr_t>(&q[0].z) % 64, 0u);
}

TEST(StructSplitter, soaos) {
  PointSoAoS q(3, 64);

  for (size_t i = 0; i < q.size(); ++i) {
    q[i].x = static_cast<float>(i);
    q[i].y = static_cast<int>(i);
    q[i].z = static_cast<double>(i);
  }

  q[0].z = 42;
  EXPECT_EQ(q[0].x, 0.0f); EXPECT_EQ(q[0].y, 0); EXPECT_EQ(q[0].z, 42.0);
  EXPECT_EQ(q[1].x, 1.0f); EXPECT_EQ(q[1].y, 1); EXPECT_EQ(q[1].z, 1.0);
  EXPECT_EQ(q[2].x, 2.0f); EXPECT_EQ(q[2].y, 2); EXPECT_EQ(q[2].z, 2.0);

  // Expected layout: xxx zyzyzy. x is in its own array of SubPoint<0>, so consecutive x
  // values are one float apart.
  for (size_t i = 0; i + 1 < q.size(); ++i) {
    EXPECT_EQ(&q[i + 1].x, &q[i].x + 1) << "x not contiguous at index " << i;
  }

  // z and y share an array of SubPoint<2, 1>: z precedes y within each element, and both
  // advance by one struct (including padding) per element.
  constexpr size_t stride = sizeof(SubPoint<2, 1>);
  for (size_t i = 0; i < q.size(); ++i) {
    EXPECT_LT(addr(q[i].z), addr(q[i].y)) << "z not before y at index " << i;
  }
  for (size_t i = 0; i + 1 < q.size(); ++i) {
    EXPECT_EQ(addr(q[i + 1].z), addr(q[i].z) + stride) << "z stride wrong at index " << i;
    EXPECT_EQ(addr(q[i + 1].y), addr(q[i].y) + stride) << "y stride wrong at index " << i;
  }

  // The two partitions come from separate allocations, so their relative order in memory is
  // not guaranteed; only check that they don't overlap.
  const auto *x_begin = addr(q[0].x), *x_end = x_begin + q.size() * sizeof(float);
  const auto *zy_begin = addr(q[0].z), *zy_end = zy_begin + q.size() * stride;
  EXPECT_TRUE(x_end <= zy_begin || zy_end <= x_begin) << "x and zy partitions overlap";

  EXPECT_EQ(reinterpret_cast<std::uintptr_t>(&q[0].x) % 64, 0u);
  EXPECT_EQ(reinterpret_cast<std::uintptr_t>(&q[0].z) % 64, 0u);
}

TEST(StructSplitter, to_string) {
  PointSoA q(3, 64);
  EXPECT_EQ(PointAoS::to_string(), "PartitionedContainer120");
  EXPECT_EQ(PointSoA::to_string(), "PartitionedContainer0_1_2");
  EXPECT_EQ(PointSoAoS::to_string(), "PartitionedContainer0_21");
}
