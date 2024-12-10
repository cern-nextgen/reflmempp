#ifndef AOSOA_H
#define AOSOA_H

#include "soa.h"
#include <algorithm>
#include <array>
#include <numeric>
#include <span>
#include <vector>

template <typename T, size_t N, size_t NChunks>
struct AoSoARaw {
private:
  struct SoA {
    alignas(64) T *x1, *x2;
  };
  alignas(64) SoA *m_v1, *m_v2, *m_vout;
  alignas(64) T *m_out;

public:
  std::span<SoA> v1, v2, vout;
  std::span<T> out;

  AoSoARaw() {
    m_v1 = new SoA[NChunks];
    m_v2 = new SoA[NChunks];
    m_vout = new SoA[NChunks];
    m_out = new T[N];

    v1 = std::span(m_v1, NChunks);
    v2 = std::span(m_v2, NChunks);
    vout = std::span(m_vout, NChunks);
    out = std::span(m_out, N);
    std::fill(out.begin(), out.end(), 0);

    auto allocate_and_generate = [](auto &container, size_t chunk_index, size_t offset) {
      container.x1 = new T[N / NChunks];
      container.x2 = new T[N / NChunks];

      std::generate(container.x1, container.x1 + (N / NChunks),
                    [n = chunk_index * (N / NChunks) + offset]() mutable { return n++; });
      std::generate(container.x2, container.x2 + (N / NChunks),
                    [n = chunk_index * (N / NChunks) + offset]() mutable { return 1 + n++; });
    };

    for (size_t i = 0; i < NChunks; ++i) {
      allocate_and_generate(m_v1[i], i, 0);
      allocate_and_generate(m_v2[i], i, 1);
      m_vout[i].x1 = new T[N / NChunks];
      m_vout[i].x2 = new T[N / NChunks];
    }
  }

  ~AoSoARaw() {
    for (size_t i = 0; i < NChunks; ++i) {
      delete[] m_v1[i].x1;
      delete[] m_v1[i].x2;
      delete[] m_v2[i].x1;
      delete[] m_v2[i].x2;
      delete[] m_vout[i].x1;
      delete[] m_vout[i].x2;
    }
    delete[] m_v1;
    delete[] m_v2;
    delete[] m_vout;
    delete[] m_out;
  }
};

template <typename T, size_t N, size_t NChunks>
struct AoSoAArr {
private:
  struct SoA {
    alignas(64) std::array<T, N / NChunks> x1, x2;
  };
  alignas(64) std::array<SoA, NChunks> *m_v1, *m_v2, *m_vout;
  alignas(64) std::array<T, N> *m_out;

public:
  std::span<SoA> v1, v2, vout;
  std::span<T> out;

  AoSoAArr() {
    m_v1 = new std::array<SoA, NChunks>;
    m_v2 = new std::array<SoA, NChunks>;
    m_vout = new std::array<SoA, NChunks>;
    m_out = new std::array<T, N>;

    v1 = std::span(m_v1->data(), NChunks);
    v2 = std::span(m_v2->data(), NChunks);
    vout = std::span(m_vout->data(), NChunks);
    out = std::span(m_out->data(), N);
    std::fill(out.begin(), out.end(), 0);

    auto generate_data = [](auto &container, size_t chunk_index, size_t offset) {
      std::generate(container.x1.begin(), container.x1.end(),
                    [n = chunk_index * (N / NChunks) + offset]() mutable { return n++; });
      std::generate(container.x2.begin(), container.x2.end(),
                    [n = chunk_index * (N / NChunks) + offset]() mutable { return 1 + n++; });
    };

    for (size_t i = 0; i < NChunks; ++i) {
      generate_data(v1[i], i, 0);
      generate_data(v2[i], i, 1);
    }
  }

  ~AoSoAArr() {
    delete m_v1;
    delete m_v2;
    delete m_vout;
    delete m_out;
  }
};

template <typename T, size_t N, size_t NChunks>
struct AoSoAVec {
  struct SoA {
    alignas(64) std::vector<T> x1, x2;
  };
  alignas(64) std::vector<SoA> v1, v2, vout;
  alignas(64) std::vector<T> out;

  AoSoAVec() {
    out.resize(N);
    std::fill(out.begin(), out.end(), 0);

    v1.resize(NChunks);
    v2.resize(NChunks);
    vout.resize(NChunks);

    auto generate_data = [](auto &container, size_t chunk_index, size_t offset) {
      std::generate_n(std::back_inserter(container.x1), N / NChunks,
                      [n = chunk_index * (N / NChunks) + offset]() mutable { return n++; });
      std::generate_n(std::back_inserter(container.x2), N / NChunks,
                      [n = chunk_index * (N / NChunks) + offset]() mutable { return 1 + n++; });
    };

    for (size_t i = 0; i < NChunks; ++i) {
      generate_data(v1[i], i, 0);
      generate_data(v2[i], i, 1);
      vout[i].x1.resize(N / NChunks);
      vout[i].x2.resize(N / NChunks);
    }
  }
};

#endif // AOSOA_H
