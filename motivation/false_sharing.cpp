#include "aos.h"
#include "aosoa.h"

#include <algorithm>
#include <array>
#include <benchmark/benchmark.h>
#include <chrono>
#include <cmath>
#include <iostream>
#include <numeric>
#include <ranges>
#include <span>
#include <vector>

#include <omp.h>
// #include <likwid-marker.h>

using Clock = std::chrono::steady_clock;
using fsecs = std::chrono::duration<double, std::chrono::seconds::period>;

constexpr size_t container_size = 10e6;
constexpr size_t chunk_size = 1;

/*
 * AoS
 */

template <typename Container>
static void BM_FlatAoS(benchmark::State &state) {
  Container data;
  auto &v1 = data.v1;
  auto &v2 = data.v2;
  auto &out = data.out;

  for (const auto &&_ : state) {
    auto start = Clock::now();
#pragma omp parallel for num_threads(state.range(0)) schedule(static, 1)
    for (size_t i = 0; i < v1.size(); ++i) {
      // printf("tid:%d i: %lu\n", omp_get_thread_num(), i);
      out[i] = (v2[i].x1 - v1[i].x1) * (v2[i].x1 - v1[i].x1) + (v2[i].x2 - v1[i].x2) * (v2[i].x2 - v1[i].x2);
    }
    auto end = Clock::now();

    auto elapsed_seconds = std::chrono::duration_cast<fsecs>(end - start);
    state.SetIterationTime(elapsed_seconds.count());
  }

  state.counters["size"] = out.size();
  state.counters["num_threads"] = state.range(0);
}

template <typename Container>
static void BM_NestedAoS(benchmark::State &state) {
  Container data;
  auto &v1 = data.v1;
  auto &v2 = data.v2;
  auto &vout = data.vout;

  for (const auto &&_ : state) {
    auto start = Clock::now();
#pragma omp parallel for num_threads(state.range(0)) schedule(static, 1)
    for (size_t i = 0; i < v1.size(); ++i) {
      vout[i].x1 = (v2[i].x1 - v1[i].x1) * (v2[i].x1 - v1[i].x1);
      vout[i].x2 = (v2[i].x2 - v1[i].x2) * (v2[i].x2 - v1[i].x2);
    }
    auto end = Clock::now();

    auto elapsed_seconds = std::chrono::duration_cast<fsecs>(end - start);
    state.SetIterationTime(elapsed_seconds.count());
  }

  state.counters["size"] = vout.size();
  state.counters["num_threads"] = state.range(0);
}
/*
 * AoSoA
 */

template <template <typename, size_t, size_t> typename Container, typename T, size_t N, size_t Sz>
void BM_FlatAoSoA(benchmark::State &state) {
  constexpr size_t nChunks = N / Sz;
  Container<T, N, nChunks> data;

  auto &out = data.out;
  for (const auto &&_ : state) {
    auto start = Clock::now();
#pragma omp parallel for num_threads(state.range(0)) schedule(static, 1)
    for (size_t i = 0; i < nChunks; ++i) {
      auto &v1 = data.v1[i];
      auto &v2 = data.v2[i];
      size_t out_index = i * Sz;
      for (size_t j = 0; j < Sz; ++j) {
        out[out_index + j] =
            (v2.x1[j] - v1.x1[j]) * (v2.x1[j] - v1.x1[j]) + (v2.x2[j] - v1.x2[j]) * (v2.x2[j] - v1.x2[j]);
      }
    }
    auto end = Clock::now();

    auto elapsed_seconds = std::chrono::duration_cast<fsecs>(end - start);
    state.SetIterationTime(elapsed_seconds.count());
  }

  state.counters["total_size"] = N;
  state.counters["num_chunks"] = nChunks;
  state.counters["chunk_size"] = Sz;
  state.counters["num_threads"] = state.range(0);
}

template <template <typename, size_t, size_t> typename Container, typename T, size_t N, size_t Sz>
void BM_NestedAoSoA(benchmark::State &state) {
  constexpr size_t nChunks = N / Sz;
  Container<T, N, nChunks> data;

  for (const auto &&_ : state) {
    auto start = Clock::now();
#pragma omp parallel for num_threads(state.range(0)) schedule(static, 1)
    for (size_t i = 0; i < nChunks; ++i) {
      auto &v1 = data.v1[i];
      auto &v2 = data.v2[i];
      auto &vout = data.vout[i];
      size_t out_index = i * Sz;
      for (size_t j = 0; j < Sz; ++j) {
        vout.x1[j] = (v2.x1[j] - v1.x1[j]) * (v2.x1[j] - v1.x1[j]);
        vout.x2[j] = (v2.x2[j] - v1.x2[j]) * (v2.x2[j] - v1.x2[j]);
      }
    }
    auto end = Clock::now();

    auto elapsed_seconds = std::chrono::duration_cast<fsecs>(end - start);
    state.SetIterationTime(elapsed_seconds.count());
  }

  state.counters["total_size"] = N;
  state.counters["num_chunks"] = nChunks;
  state.counters["chunk_size"] = Sz;
  state.counters["num_threads"] = state.range(0);
}

#define BENCHMARK_ARGS RangeMultiplier(2)->Range(2, 16)->Unit(benchmark::kMillisecond);

#define BENCHMARK_ALL_LAYOUTS(OP, TYPE)                                                                                \
  BENCHMARK_TEMPLATE(BM_##OP##AoS, AoS2Raw<TYPE, container_size>)->BENCHMARK_ARGS;                                     \
  BENCHMARK_TEMPLATE(BM_##OP##AoSoA, AoSoARaw, TYPE, container_size, chunk_size)->BENCHMARK_ARGS;
  // BENCHMARK_TEMPLATE(BM_##OP##AoSoA, AoSoAVec, TYPE, container_size, chunk_size)->BENCHMARK_ARGS;                      \
  // BENCHMARK_TEMPLATE(BM_##OP##AoS, AoS2Vec<TYPE, container_size>)->BENCHMARK_ARGS;
  // BENCHMARK_TEMPLATE(BM_##OP##AoS, AoS2Arr<TYPE, container_size>)->BENCHMARK_ARGS;                                     \
  // BENCHMARK_TEMPLATE(BM_##OP##AoSoA, AoSoAArr, TYPE, container_size, chunk_size)->BENCHMARK_ARGS;                      \

// BENCHMARK_ALL_LAYOUTS(Flat, float)
// BENCHMARK_ALL_LAYOUTS(Nested, float)
BENCHMARK_ALL_LAYOUTS(Flat, double)
BENCHMARK_ALL_LAYOUTS(Nested, double)

BENCHMARK_MAIN();
