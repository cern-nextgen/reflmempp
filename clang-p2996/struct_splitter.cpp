#include "struct_splitter.h"

struct Point {
  float &x;
  int &y;
  double &z;

  double sum() const { return x + y + z; };
};

template <auto Members>
struct SubPoint;

consteval {
  SplitStruct<Point, SubPoint>(SplitOp({0}), SplitOp({1}), SplitOp({2}));
}
using PointSoA =
    PartitionedContainer<Point, SubPoint<SplitOp({0}).data()>, SubPoint<SplitOp({1}).data()>, SubPoint<SplitOp({2}).data()>>;

int main() {
    PointSoA q(3, 64);
    for (size_t i = 0; i < q.size(); ++i) {
        q[i].x = static_cast<float>(i);
        q[i].y = static_cast<int>(i);
        q[i].z = static_cast<double>(i);
    }

    q[0].z = 42;
    std::cout << "q[0] = { x: " << q[0].x << ", y: " << q[0].y << ", z: " << q[0].z << " }" << std::endl;
    std::cout << "q[1] = { x: " << q[1].x << ", y: " << q[1].y << ", z: " << q[1].z << " }" << std::endl;
    std::cout << "q[2] = { x: " << q[2].x << ", y: " << q[2].y << ", z: " << q[2].z << " }" << std::endl;
    std::cout << "sum q[2] == " << q[2].sum() << std::endl;
  return 0;
}