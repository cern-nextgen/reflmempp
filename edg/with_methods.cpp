// SoA structure with an AoS interface using P2996 reflection and P3294 token
// injection. Each array in the SoA is allocated in a contiguous storage
// container.

#define __cpp_lib_reflection 20240815

#include "rmpp_ref.h"
#include <experimental/meta>
#include <iostream>
#include <tuple>

using namespace std::literals::string_view_literals;

// Example from LLAMA thesis Listing A.1
struct Vec {
  float x, y, z;

  void MakePlanar() & { z = 0; }

  friend std::ostream &operator<<(std::ostream &os, const Vec &obj) {
    return os << "{" << obj.x << ", " << obj.y << ", " << obj.z << "}";
  }

  // Vec() : x(0), y(0), z(0) { std::cout << "Vec default constructor called\n"; }

  // Vec(const Vec &other) : x(other.x), y(other.y), z(other.z) { std::cout << "Vec copy constructor called\n"; }

  // Vec &operator=(const Vec &other) {
  //   if (this != &other) {
  //     x = other.x;
  //     y = other.y;
  //     z = other.z;
  //     std::cout << "Vec copy assignment operator called\n";
  //   }
  //   return *this;
  // }
};

struct Particle {
  int &id;
  Vec &pos;

  void SetIdMember(int v)  { id = v; }

  // Particle(int &id, Vec &pos) : id(id), pos(pos) { std::cout << "Particle default constructor called\n"; }

  // Particle(const Particle &other) : id(other.id), pos(other.pos) { std::cout << "Particle copy constructor called\n"; }

  // Particle &operator=(const Particle &other) {
  //   if (this != &other) {
  //     id = other.id;
  //     pos = other.pos;
  //     std::cout << "Particle copy assignment operator called\n";
  //   }
  //   return *this;
  // }
};

using SoA = rmpp::vector<Particle, 64, rmpp::layout::soa, rmpp::layout::aos>;

void print_maos(auto &maos) {
  std::cout << "maos.size = " << maos.size();
  for (size_t i = 0; i != maos.size(); ++i) {
    std::cout << "\nmaos[" << i << "] = (\n";

    [:expand(nonstatic_data_members_of(^decltype(maos[i]))):] >> [&]<auto e> {
      std::cout << "\t" << name_of(e) << ": ";
      print_member(maos[i].[:e:]);
      std::cout << "\n";
    };

    [:expand(nonstatic_data_members_of(^decltype(maos[i]))):] >> [&]<auto e> {
      std::cout << "\t" << name_of(e) << ": ";
      print_member_addr(maos[i].[:e:]);
      std::cout << "\n";
    };
  }
}

int main() {
  // SoA maos{};
  SoA maos = {{{0, {1., 2., 3.}}, {1, {4., 5., 6.}}, {2, {7., 8., 9.}}}};

  std::cout << "-----------------------\n";
  print_maos(maos);

  assert(maos[0].id == 0);
  assert(maos[1].id == 1);
  assert(maos[2].id == 2);

  maos[0].SetIdMember(9);
  maos[0].pos.MakePlanar();

  std::cout << "-----------------------\n";
  print_maos(maos);

  return 0;
}
