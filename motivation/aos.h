#ifndef AOS_H
#define AOS_H

#include <algorithm>
#include <array>
#include <span>
#include <vector>

/*
 * 2 Data Members
 */

template <typename T>
struct S2 {
  T x1, x2;
};

template <typename T>
S2<T> GenerateS2(int i) {
  return S2<T>{(T)i, (T)1 + i};
}

/*
 * 4 Data Members
 */
template <typename T>
struct S4 {
  T x1, x2, x3, x4;
};

template <typename T>
S4<T> GenerateS4(int i) {
  return S4<T>{(T)i, (T)1 + i, (T)2 + i, (T)3 + i};
}

/*
 * 20 Data Members
 */

template <typename T>
struct S20 {
  T x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15, x16, x17, x18, x19, x20;
};

template <typename T>
S20<T> GenerateS20(int i) {
  return S20<T>{(T)i, (T)1 + i, (T)2 + i, (T)3 + i, (T)4 + i, (T)5 + i, (T)6 + i, (T)7 + i, (T)8 + i, (T)9 + i, 
                (T)10 + i, (T)11 + i, (T)12 + i, (T)13 + i, (T)14 + i, (T)15 + i, (T)16 + i, (T)17 + i, (T)18 + i, (T)19 + i};
}

template <typename T, size_t N, template<typename> class S, S<T> (*Generate)(int)>
struct AoSBaseRaw {
private:
  alignas(64) S<T> *m_v1, *m_v2;
  alignas(64) T *m_out;

public:
  std::span<S<T>> v1, v2;
  std::span<T> out;

  AoSBaseRaw() {
    m_v1 = new S<T>[N];
    m_v2 = new S<T>[N];
    m_out = new T[N];
    v1 = std::span(m_v1, N);
    v2 = std::span(m_v2, N);
    out = std::span(m_out, N);

    std::generate(v1.begin(), v1.end(), [i = 1]() mutable { return Generate(i++); });
    std::generate(v2.begin(), v2.end(), [i = 1]() mutable { return Generate(i++ + 1); });
    std::fill(out.begin(), out.end(), 0);
  }

  ~AoSBaseRaw() {
    delete[] m_v1;
    delete[] m_v2;
    delete[] m_out;
  }
};

template <typename T, size_t N, template<typename> class S, S<T> (*Generate)(int)>
struct AoSBaseArr {
private:
  alignas(64) std::array<S<T>, N> *m_v1, *m_v2;
  alignas(64) std::array<T, N> *m_out;

public:
  std::span<S<T>> v1, v2;
  std::span<T> out;

  AoSBaseArr() {
    m_v1 = new std::array<S<T>, N>;
    m_v2 = new std::array<S<T>, N>;
    m_out = new std::array<T, N>;
    v1 = std::span(m_v1->data(), N);
    v2 = std::span(m_v2->data(), N);
    out = std::span(m_out->data(), N);

    std::generate(v1.begin(), v1.end(), [i = 1]() mutable { return Generate(i++); });
    std::generate(v2.begin(), v2.end(), [i = 1]() mutable { return Generate(i++ + 1); });
    std::fill(out.begin(), out.end(), 0);
  }

  ~AoSBaseArr() {
    delete m_v1;
    delete m_v2;
    delete m_out;
  }
};

template <typename T, size_t N, template<typename> class S, S<T> (*Generate)(int)>
struct AoSBaseVec {
  alignas(64) std::vector<S<T>> v1, v2;
  alignas(64) std::vector<T> out;

  AoSBaseVec() {
    out.resize(N);

    std::generate_n(std::back_inserter(v1), N, [i = 1]() mutable { return Generate(i++); });
    std::generate_n(std::back_inserter(v2), N, [i = 1]() mutable { return Generate(i++ + 1); });
    std::fill(out.begin(), out.end(), 0);
  }
};

// Specializations for S2 (100%)
template <typename T, size_t N>
using AoS2Raw = AoSBaseRaw<T, N, S2, GenerateS2>;

template <typename T, size_t N>
using AoS2Arr = AoSBaseArr<T, N, S2, GenerateS2>;

template <typename T, size_t N>
using AoS2Vec = AoSBaseVec<T, N, S2, GenerateS2>;

// Specializations for S4 (50%)
template <typename T, size_t N>
using AoS4Raw = AoSBaseRaw<T, N, S4, GenerateS4>;

template <typename T, size_t N>
using AoS4Arr = AoSBaseArr<T, N, S4, GenerateS4>;

template <typename T, size_t N>
using AoS4Vec = AoSBaseVec<T, N, S4, GenerateS4>;

// Specializations for S8 (25%)
template <typename T>
struct S8 {
  T x1, x2, x3, x4, x5, x6, x7, x8;
};

template <typename T>
S8<T> GenerateS8(int i) {
  return S8<T>{(T)i, (T)1 + i, (T)2 + i, (T)3 + i, (T)4 + i, (T)5 + i, (T)6 + i, (T)7 + i};
}

template <typename T, size_t N>
using AoS8Raw = AoSBaseRaw<T, N, S8, GenerateS8>;

template <typename T, size_t N>
using AoS8Arr = AoSBaseArr<T, N, S8, GenerateS8>;

template <typename T, size_t N>
using AoS8Vec = AoSBaseVec<T, N, S8, GenerateS8>;

// Specializations for S20 (10%)
template <typename T, size_t N>
using AoS20Raw = AoSBaseRaw<T, N, S20, GenerateS20>;

template <typename T, size_t N>
using AoS20Arr = AoSBaseArr<T, N, S20, GenerateS20>;

template <typename T, size_t N>
using AoS20Vec = AoSBaseVec<T, N, S20, GenerateS20>;

#endif // AOS_H
