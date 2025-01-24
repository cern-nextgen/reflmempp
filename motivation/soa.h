#ifndef SOA_H
#define SOA_H

#include <algorithm>
#include <array>
#include <span>
#include <vector>

template <typename T, size_t N>
void initialize_and_generate(T *&x1r, std::span<T> &span, int start) {
  x1r = new T[N];
  span = std::span(x1r, N);
  std::generate(span.begin(), span.end(), [i = start]() mutable { return i++; });
}

template <typename T, size_t N>
void initialize_and_generate(std::array<T, N> *&x1r, std::span<T> &span, int start) {
  x1r = new std::array<T, N>;
  span = std::span(x1r->data(), N);
  std::generate(span.begin(), span.end(), [i = start]() mutable { return i++; });
}

template <typename T, size_t N>
void initialize_and_generate(std::vector<T> &vec, int start) {
  vec.resize(N);
  std::generate_n(vec.begin(), N, [i = start]() mutable { return i++; });
}

/*
 * 2 Data Members
 */

template <typename T, size_t N>
struct SoA2Raw {
private:
  alignas(64) T *m_v1_x1, *m_v1_x2, *m_v2_x1, *m_v2_x2, *m_out;

public:
  std::span<T> v1_x1, v1_x2, v2_x1, v2_x2, out;

  SoA2Raw() {
    initialize_and_generate<T, N>(m_v1_x1, v1_x1, 1);
    initialize_and_generate<T, N>(m_v1_x2, v1_x2, 2);
    initialize_and_generate<T, N>(m_v2_x1, v2_x1, 2);
    initialize_and_generate<T, N>(m_v2_x2, v2_x2, 3);
    initialize_and_generate<T, N>(m_out, out, 0);
    std::fill(out.begin(), out.end(), 0);
  }

  ~SoA2Raw() {
    delete[] m_v1_x1;
    delete[] m_v1_x2;
    delete[] m_v2_x1;
    delete[] m_v2_x2;
    delete[] m_out;
  }
};

template <typename T, size_t N>
struct SoA2Arr {
private:
  alignas(64) std::array<T, N> *m_v1_x1, *m_v1_x2, *m_v2_x1, *m_v2_x2, *m_out;

public:
  std::span<T> v1_x1, v1_x2, v2_x1, v2_x2, out;

  SoA2Arr() {
    initialize_and_generate<T, N>(m_v1_x1, v1_x1, 1);
    initialize_and_generate<T, N>(m_v1_x2, v1_x2, 2);
    initialize_and_generate<T, N>(m_v2_x1, v2_x1, 2);
    initialize_and_generate<T, N>(m_v2_x2, v2_x2, 3);
    initialize_and_generate<T, N>(m_out, out, 0);
    std::fill(out.begin(), out.end(), 0);
  }

  ~SoA2Arr() {
    delete m_v1_x1;
    delete m_v1_x2;
    delete m_v2_x1;
    delete m_v2_x2;
    delete m_out;
  }
};

template <typename T, size_t N>
struct SoA2Vec {
  alignas(64) std::vector<T> v1_x1, v1_x2, v2_x1, v2_x2, out;

  SoA2Vec() {
    out.resize(N);
    std::fill(out.begin(), out.end(), 0);

    initialize_and_generate<T, N>(v1_x1, 0);
    initialize_and_generate<T, N>(v1_x2, 1);
    initialize_and_generate<T, N>(v2_x1, 1);
    initialize_and_generate<T, N>(v2_x2, 2);
  }
};

/*
 * 4 Data Members
 */

template <typename T, size_t N>
struct SoA4Raw {
private:
  alignas(64) T *m_v1_x1, *m_v1_x2, *m_v1_x3, *m_v1_x4, *m_v2_x1, *m_v2_x2, *m_v2_x3, *m_v2_x4, *m_out;

public:
  std::span<T> v1_x1, v1_x2, v1_x3, v1_x4, v2_x1, v2_x2, v2_x3, v2_x4, out;

  SoA4Raw() {
    initialize_and_generate<T, N>(m_v1_x1, v1_x1, 1);
    initialize_and_generate<T, N>(m_v1_x2, v1_x2, 2);
    initialize_and_generate<T, N>(m_v1_x3, v1_x3, 3);
    initialize_and_generate<T, N>(m_v1_x4, v1_x4, 4);
    initialize_and_generate<T, N>(m_v2_x1, v2_x1, 2);
    initialize_and_generate<T, N>(m_v2_x2, v2_x2, 3);
    initialize_and_generate<T, N>(m_v2_x3, v2_x3, 4);
    initialize_and_generate<T, N>(m_v2_x4, v2_x4, 5);

    initialize_and_generate<T, N>(m_out, out, 0);
    std::fill(out.begin(), out.end(), 0);
  }

  ~SoA4Raw() {
    delete[] m_v1_x1;
    delete[] m_v1_x2;
    delete[] m_v1_x3;
    delete[] m_v1_x4;
    delete[] m_v2_x1;
    delete[] m_v2_x2;
    delete[] m_v2_x3;
    delete[] m_v2_x4;
    delete[] m_out;
  }
};

template <typename T, size_t N>
struct SoA4Arr {
private:
  alignas(64) std::array<T, N> *m_v1_x1, *m_v1_x2, *m_v1_x3, *m_v1_x4, *m_v2_x1, *m_v2_x2, *m_v2_x3, *m_v2_x4, *m_out;

public:
  std::span<T> v1_x1, v1_x2, v1_x3, v1_x4, v2_x1, v2_x2, v2_x3, v2_x4, out;

  SoA4Arr() {
    initialize_and_generate<T, N>(m_v1_x1, v1_x1, 1);
    initialize_and_generate<T, N>(m_v1_x2, v1_x2, 2);
    initialize_and_generate<T, N>(m_v1_x3, v1_x3, 3);
    initialize_and_generate<T, N>(m_v1_x4, v1_x4, 4);
    initialize_and_generate<T, N>(m_v2_x1, v2_x1, 2);
    initialize_and_generate<T, N>(m_v2_x2, v2_x2, 3);
    initialize_and_generate<T, N>(m_v2_x3, v2_x3, 4);
    initialize_and_generate<T, N>(m_v2_x4, v2_x4, 5);

    initialize_and_generate<T, N>(m_out, out, 0);
    std::fill(out.begin(), out.end(), 0);
  }

  ~SoA4Arr() {
    delete m_v1_x1;
    delete m_v1_x2;
    delete m_v1_x3;
    delete m_v1_x4;
    delete m_v2_x1;
    delete m_v2_x2;
    delete m_v2_x3;
    delete m_v2_x4;
    delete m_out;
  }
};

template <typename T, size_t N>
struct SoA4Vec {
  alignas(64) std::vector<T> v1_x1, v1_x2, v1_x3, v1_x4, v2_x1, v2_x2, v2_x3, v2_x4, out;

  SoA4Vec() {
    out.resize(N);
    std::fill(out.begin(), out.end(), 0);

    initialize_and_generate<T, N>(v1_x1, 1);
    initialize_and_generate<T, N>(v1_x2, 2);
    initialize_and_generate<T, N>(v1_x3, 3);
    initialize_and_generate<T, N>(v1_x4, 4);
    initialize_and_generate<T, N>(v2_x1, 2);
    initialize_and_generate<T, N>(v2_x2, 3);
    initialize_and_generate<T, N>(v2_x3, 4);
    initialize_and_generate<T, N>(v2_x4, 5);
  }
};

/*
 * 8 Data Members
 */

template <typename T, size_t N>
struct SoA8Raw {
private:
  alignas(64) T *m_v1_x1, *m_v1_x2, *m_v1_x3, *m_v1_x4, *m_v1_x5, *m_v1_x6, *m_v1_x7, *m_v1_x8, *m_v2_x1, *m_v2_x2,
      *m_v2_x3, *m_v2_x4, *m_v2_x5, *m_v2_x6, *m_v2_x7, *m_v2_x8, *m_out;

public:
  std::span<T> v1_x1, v1_x2, v1_x3, v1_x4, v1_x5, v1_x6, v1_x7, v1_x8, v2_x1, v2_x2, v2_x3, v2_x4, v2_x5, v2_x6, v2_x7,
      v2_x8, out;

  SoA8Raw() {
    initialize_and_generate<T, N>(m_v1_x1, v1_x1, 1);
    initialize_and_generate<T, N>(m_v1_x2, v1_x2, 2);
    initialize_and_generate<T, N>(m_v1_x3, v1_x3, 3);
    initialize_and_generate<T, N>(m_v1_x4, v1_x4, 4);
    initialize_and_generate<T, N>(m_v1_x5, v1_x5, 5);
    initialize_and_generate<T, N>(m_v1_x6, v1_x6, 6);
    initialize_and_generate<T, N>(m_v1_x7, v1_x7, 7);
    initialize_and_generate<T, N>(m_v1_x8, v1_x8, 8);
    initialize_and_generate<T, N>(m_v2_x1, v2_x1, 2);
    initialize_and_generate<T, N>(m_v2_x2, v2_x2, 3);
    initialize_and_generate<T, N>(m_v2_x3, v2_x3, 4);
    initialize_and_generate<T, N>(m_v2_x4, v2_x4, 5);
    initialize_and_generate<T, N>(m_v2_x5, v2_x5, 6);
    initialize_and_generate<T, N>(m_v2_x6, v2_x6, 7);
    initialize_and_generate<T, N>(m_v2_x7, v2_x7, 8);
    initialize_and_generate<T, N>(m_v2_x8, v2_x8, 9);

    initialize_and_generate<T, N>(m_out, out, 0);
    std::fill(out.begin(), out.end(), 0);
  }

  ~SoA8Raw() {
    delete[] m_v1_x1;
    delete[] m_v1_x2;
    delete[] m_v1_x3;
    delete[] m_v1_x4;
    delete[] m_v1_x5;
    delete[] m_v1_x6;
    delete[] m_v1_x7;
    delete[] m_v1_x8;
    delete[] m_v2_x1;
    delete[] m_v2_x2;
    delete[] m_v2_x3;
    delete[] m_v2_x4;
    delete[] m_v2_x5;
    delete[] m_v2_x6;
    delete[] m_v2_x7;
    delete[] m_v2_x8;
    delete[] m_out;
  }
};

template <typename T, size_t N>
struct SoA8Arr {
private:
  alignas(64) std::array<T, N> *m_v1_x1, *m_v1_x2, *m_v1_x3, *m_v1_x4, *m_v1_x5, *m_v1_x6, *m_v1_x7, *m_v1_x8, *m_v2_x1,
      *m_v2_x2, *m_v2_x3, *m_v2_x4, *m_v2_x5, *m_v2_x6, *m_v2_x7, *m_v2_x8, *m_out;

public:
  std::span<T> v1_x1, v1_x2, v1_x3, v1_x4, v1_x5, v1_x6, v1_x7, v1_x8, v2_x1, v2_x2, v2_x3, v2_x4, v2_x5, v2_x6, v2_x7,
      v2_x8, out;

  SoA8Arr() {
    initialize_and_generate<T, N>(m_v1_x1, v1_x1, 1);
    initialize_and_generate<T, N>(m_v1_x2, v1_x2, 2);
    initialize_and_generate<T, N>(m_v1_x3, v1_x3, 3);
    initialize_and_generate<T, N>(m_v1_x4, v1_x4, 4);
    initialize_and_generate<T, N>(m_v1_x5, v1_x5, 5);
    initialize_and_generate<T, N>(m_v1_x6, v1_x6, 6);
    initialize_and_generate<T, N>(m_v1_x7, v1_x7, 7);
    initialize_and_generate<T, N>(m_v1_x8, v1_x8, 8);
    initialize_and_generate<T, N>(m_v2_x1, v2_x1, 2);
    initialize_and_generate<T, N>(m_v2_x2, v2_x2, 3);
    initialize_and_generate<T, N>(m_v2_x3, v2_x3, 4);
    initialize_and_generate<T, N>(m_v2_x4, v2_x4, 5);
    initialize_and_generate<T, N>(m_v2_x5, v2_x5, 6);
    initialize_and_generate<T, N>(m_v2_x6, v2_x6, 7);
    initialize_and_generate<T, N>(m_v2_x7, v2_x7, 8);
    initialize_and_generate<T, N>(m_v2_x8, v2_x8, 9);

    initialize_and_generate<T, N>(m_out, out, 0);
    std::fill(out.begin(), out.end(), 0);
  }

  ~SoA8Arr() {
    delete m_v1_x1;
    delete m_v1_x2;
    delete m_v1_x3;
    delete m_v1_x4;
    delete m_v1_x5;
    delete m_v1_x6;
    delete m_v1_x7;
    delete m_v1_x8;
    delete m_v2_x1;
    delete m_v2_x2;
    delete m_v2_x3;
    delete m_v2_x4;
    delete m_v2_x5;
    delete m_v2_x6;
    delete m_v2_x7;
    delete m_v2_x8;
    delete m_out;
  }
};

template <typename T, size_t N>
struct SoA8Vec {
  alignas(64) std::vector<T> v1_x1, v1_x2, v1_x3, v1_x4, v1_x5, v1_x6, v1_x7, v1_x8, v2_x1, v2_x2, v2_x3, v2_x4, v2_x5,
      v2_x6, v2_x7, v2_x8, out;

  SoA8Vec() {
    out.resize(N);
    std::fill(out.begin(), out.end(), 0);

    initialize_and_generate<T, N>(v1_x1, 1);
    initialize_and_generate<T, N>(v1_x2, 2);
    initialize_and_generate<T, N>(v1_x3, 3);
    initialize_and_generate<T, N>(v1_x4, 4);
    initialize_and_generate<T, N>(v1_x5, 5);
    initialize_and_generate<T, N>(v1_x6, 6);
    initialize_and_generate<T, N>(v1_x7, 7);
    initialize_and_generate<T, N>(v1_x8, 8);

    initialize_and_generate<T, N>(v2_x1, 2);
    initialize_and_generate<T, N>(v2_x2, 3);
    initialize_and_generate<T, N>(v2_x3, 4);
    initialize_and_generate<T, N>(v2_x4, 5);
    initialize_and_generate<T, N>(v2_x5, 6);
    initialize_and_generate<T, N>(v2_x6, 7);
    initialize_and_generate<T, N>(v2_x7, 8);
    initialize_and_generate<T, N>(v2_x8, 9);
  }
};

/*
 * 20 Data Members
 */

template <typename T, size_t N>
struct SoA20Raw {
private:
  alignas(64) T *m_v1_x1, *m_v1_x2, *m_v1_x3, *m_v1_x4, *m_v1_x5, *m_v1_x6, *m_v1_x7, *m_v1_x8, *m_v1_x9, *m_v1_x10,
      *m_v2_x1, *m_v2_x2, *m_v2_x3, *m_v2_x4, *m_v2_x5, *m_v2_x6, *m_v2_x7, *m_v2_x8, *m_v2_x9, *m_v2_x10, *m_out;

public:
  std::span<T> v1_x1, v1_x2, v1_x3, v1_x4, v1_x5, v1_x6, v1_x7, v1_x8, v1_x9, v1_x10, v2_x1, v2_x2, v2_x3, v2_x4, v2_x5,
      v2_x6, v2_x7, v2_x8, v2_x9, v2_x10, out;

  SoA20Raw() {
    initialize_and_generate<T, N>(m_v1_x1, v1_x1, 1);
    initialize_and_generate<T, N>(m_v1_x2, v1_x2, 2);
    initialize_and_generate<T, N>(m_v1_x3, v1_x3, 3);
    initialize_and_generate<T, N>(m_v1_x4, v1_x4, 4);
    initialize_and_generate<T, N>(m_v1_x5, v1_x5, 5);
    initialize_and_generate<T, N>(m_v1_x6, v1_x6, 6);
    initialize_and_generate<T, N>(m_v1_x7, v1_x7, 7);
    initialize_and_generate<T, N>(m_v1_x8, v1_x8, 8);
    initialize_and_generate<T, N>(m_v1_x9, v1_x9, 9);
    initialize_and_generate<T, N>(m_v1_x10, v1_x10, 10);

    initialize_and_generate<T, N>(m_v2_x1, v2_x1, 2);
    initialize_and_generate<T, N>(m_v2_x2, v2_x2, 3);
    initialize_and_generate<T, N>(m_v2_x3, v2_x3, 4);
    initialize_and_generate<T, N>(m_v2_x4, v2_x4, 5);
    initialize_and_generate<T, N>(m_v2_x5, v2_x5, 6);
    initialize_and_generate<T, N>(m_v2_x6, v2_x6, 7);
    initialize_and_generate<T, N>(m_v2_x7, v2_x7, 8);
    initialize_and_generate<T, N>(m_v2_x8, v2_x8, 9);
    initialize_and_generate<T, N>(m_v2_x9, v2_x9, 10);
    initialize_and_generate<T, N>(m_v2_x10, v2_x10, 11);

    initialize_and_generate<T, N>(m_out, out, 0);
    std::fill(out.begin(), out.end(), 0);
  }

  ~SoA20Raw() {
    delete[] m_v1_x1;
    delete[] m_v1_x2;
    delete[] m_v1_x3;
    delete[] m_v1_x4;
    delete[] m_v1_x5;
    delete[] m_v1_x6;
    delete[] m_v1_x7;
    delete[] m_v1_x8;
    delete[] m_v1_x9;
    delete[] m_v1_x10;
    delete[] m_v2_x1;
    delete[] m_v2_x2;
    delete[] m_v2_x3;
    delete[] m_v2_x4;
    delete[] m_v2_x5;
    delete[] m_v2_x6;
    delete[] m_v2_x7;
    delete[] m_v2_x8;
    delete[] m_v2_x9;
    delete[] m_v2_x10;
    delete[] m_out;
  }
};

template <typename T, size_t N>
struct SoA20Arr {
private:
  alignas(64) std::array<T, N> *m_v1_x1, *m_v1_x2, *m_v1_x3, *m_v1_x4, *m_v1_x5, *m_v1_x6, *m_v1_x7, *m_v1_x8, *m_v1_x9,
      *m_v1_x10, *m_v2_x1, *m_v2_x2, *m_v2_x3, *m_v2_x4, *m_v2_x5, *m_v2_x6, *m_v2_x7, *m_v2_x8, *m_v2_x9, *m_v2_x10,
      *m_out;

public:
  std::span<T> v1_x1, v1_x2, v1_x3, v1_x4, v1_x5, v1_x6, v1_x7, v1_x8, v1_x9, v1_x10, v2_x1, v2_x2, v2_x3, v2_x4, v2_x5,
      v2_x6, v2_x7, v2_x8, v2_x9, v2_x10, out;

  SoA20Arr() {
    initialize_and_generate<T, N>(m_v1_x1, v1_x1, 1);
    initialize_and_generate<T, N>(m_v1_x2, v1_x2, 2);
    initialize_and_generate<T, N>(m_v1_x3, v1_x3, 3);
    initialize_and_generate<T, N>(m_v1_x4, v1_x4, 4);
    initialize_and_generate<T, N>(m_v1_x5, v1_x5, 5);
    initialize_and_generate<T, N>(m_v1_x6, v1_x6, 6);
    initialize_and_generate<T, N>(m_v1_x7, v1_x7, 7);
    initialize_and_generate<T, N>(m_v1_x8, v1_x8, 8);
    initialize_and_generate<T, N>(m_v1_x9, v1_x9, 9);
    initialize_and_generate<T, N>(m_v1_x10, v1_x10, 10);

    initialize_and_generate<T, N>(m_v2_x1, v2_x1, 2);
    initialize_and_generate<T, N>(m_v2_x2, v2_x2, 3);
    initialize_and_generate<T, N>(m_v2_x3, v2_x3, 4);
    initialize_and_generate<T, N>(m_v2_x4, v2_x4, 5);
    initialize_and_generate<T, N>(m_v2_x5, v2_x5, 6);
    initialize_and_generate<T, N>(m_v2_x6, v2_x6, 7);
    initialize_and_generate<T, N>(m_v2_x7, v2_x7, 8);
    initialize_and_generate<T, N>(m_v2_x8, v2_x8, 9);
    initialize_and_generate<T, N>(m_v2_x9, v2_x9, 10);
    initialize_and_generate<T, N>(m_v2_x10, v2_x10, 11);

    initialize_and_generate<T, N>(m_out, out, 0);
    std::fill(out.begin(), out.end(), 0);
  }

  ~SoA20Arr() {
    delete m_v1_x1;
    delete m_v1_x2;
    delete m_v1_x3;
    delete m_v1_x4;
    delete m_v1_x5;
    delete m_v1_x6;
    delete m_v1_x7;
    delete m_v1_x8;
    delete m_v1_x9;
    delete m_v1_x10;
    delete m_v2_x1;
    delete m_v2_x2;
    delete m_v2_x3;
    delete m_v2_x4;
    delete m_v2_x5;
    delete m_v2_x6;
    delete m_v2_x7;
    delete m_v2_x8;
    delete m_v2_x9;
    delete m_v2_x10;
    delete m_out;
  }
};

template <typename T, size_t N>
struct SoA20Vec {
  alignas(64) std::vector<T> v1_x1, v1_x2, v1_x3, v1_x4, v1_x5, v1_x6, v1_x7, v1_x8, v1_x9, v1_x10, v2_x1, v2_x2, v2_x3,
      v2_x4, v2_x5, v2_x6, v2_x7, v2_x8, v2_x9, v2_x10, out;

  SoA20Vec() {
    out.resize(N);
    std::fill(out.begin(), out.end(), 0);

    initialize_and_generate<T, N>(v1_x1, 1);
    initialize_and_generate<T, N>(v1_x2, 2);
    initialize_and_generate<T, N>(v1_x3, 3);
    initialize_and_generate<T, N>(v1_x4, 4);
    initialize_and_generate<T, N>(v1_x5, 5);
    initialize_and_generate<T, N>(v1_x6, 6);
    initialize_and_generate<T, N>(v1_x7, 7);
    initialize_and_generate<T, N>(v1_x8, 8);
    initialize_and_generate<T, N>(v1_x9, 9);
    initialize_and_generate<T, N>(v1_x10, 10);

    initialize_and_generate<T, N>(v2_x1, 2);
    initialize_and_generate<T, N>(v2_x2, 3);
    initialize_and_generate<T, N>(v2_x3, 4);
    initialize_and_generate<T, N>(v2_x4, 5);
    initialize_and_generate<T, N>(v2_x5, 6);
    initialize_and_generate<T, N>(v2_x6, 7);
    initialize_and_generate<T, N>(v2_x7, 8);
    initialize_and_generate<T, N>(v2_x8, 9);
    initialize_and_generate<T, N>(v2_x9, 10);
    initialize_and_generate<T, N>(v2_x10, 11);
  }
};

#endif // SOA_H