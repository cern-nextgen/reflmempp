#include "aos.h"
#include "soa.h"
#include <algorithm>
#include <array>
#include <benchmark/benchmark.h>
#include <cmath>
#include <iostream>
#include <numeric>
#include <random>
#include <ranges>
#include <vector>

#include <chrono>
using Clock = std::chrono::steady_clock;
using fsecs = std::chrono::duration<double, std::chrono::seconds::period>;

constexpr size_t container_size = 10e6;

std::vector<size_t> generateIndicesWithStride(int n, int stride) {
  std::vector<size_t> result;
  result.reserve(n);

  // Add elements with the given stride
  for (int start = 0; start < stride; ++start) {
    for (int i = start; i < n; i += stride) {
      result.push_back(i);
    }
  }

  return result;
}

std::vector<size_t> generateRandomIndices(int n) {
  std::vector<size_t> result(n);
  std::iota(result.begin(), result.end(), 0);
  std::mt19937 g(123);
  std::shuffle(result.begin(), result.end(), g);
  return result;
}

/*
 * AoS
 */

template <typename Container>
static void BM_MemoryBoundAoS(benchmark::State &state) {
  Container data;
  auto &v1 = data.v1;
  auto &v2 = data.v2;
  auto &out = data.vout;

  std::vector<size_t> idx;
  if (state.range(0)) {
    idx = generateRandomIndices(out.size());
  } else {
    idx = generateIndicesWithStride(out.size(), 2);
  }

  for (const auto &&_ : state) {
    auto start = Clock::now();
    std::for_each(idx.begin(), idx.end(), [&](const auto &i) {
      out[i].x1 = v1[i].x1 + v2[i].x1;
      out[i].x2 = v1[i].x2 + v2[i].x2;
      out[i].x3 = v1[i].x3 + v2[i].x3;
      out[i].x4 = v1[i].x4 + v2[i].x4;
      out[i].x5 = v1[i].x5 + v2[i].x5;
      out[i].x6 = v1[i].x6 + v2[i].x6;
      out[i].x7 = v1[i].x7 + v2[i].x7;
      out[i].x8 = v1[i].x8 + v2[i].x8;
    });
    auto end = Clock::now();

    auto elapsed_seconds = std::chrono::duration_cast<fsecs>(end - start);
    state.SetIterationTime(elapsed_seconds.count());
  }

  state.counters["size"] = out.size();
  state.counters["random"] = state.range(0);
  state.counters["stride"] = state.range(1);
}

template <typename Container>
static void BM_ComputeBoundAoS(benchmark::State &state) {
  Container data;
  auto &v1 = data.v1;
  auto &v2 = data.v2;
  auto &out = data.vout;

  std::vector<size_t> idx;
  if (state.range(0)) {
    idx = generateRandomIndices(out.size());
  } else {
    idx = generateIndicesWithStride(out.size(), 2);
  }

  auto compute = [](const auto &x1, const auto &x2) {
    return (x1 - x2) * (x2 - x1) + (x2 - x1) * (x1 - x2) + (x1 + x2) * (x2 - x1) + (x2 - x1) * (x1 + x2) +
           (x1 + x2) * (x2 + x1) + (x2 + x1) * (x1 + x2) + (x1 - x1) * (x1 - x1) + (x2 - x2) * (x2 - x2) +
           (x1 + x1) * (x1 + x1) + (x2 + x2) * (x2 + x2) + (x1 - x1) * (x1 - x1) - (x1 - x2) / (x2 - x1) -
           (x2 - x1) / (x1 - x2) - (x1 + x2) / (x2 - x1) - (x2 - x1) / (x1 + x2) - (x1 + x2) / (x2 + x1) -
           (x2 + x1) / (x1 + x2) - (x1 - x1) / (x1 - x1) - (x2 - x2) / (x2 - x2) - (x1 + x1) / (x1 + x1) -
           (x2 + x2) / (x2 + x2) - (x1 * x1) - (x1 * x1) + (x1 * x2) - (x2 * x1) + (x2 * x1) - (x1 * x2) + (x1 * x2) -
           (x2 * x1) + (x2 * x1) - (x1 * x2) + (x1 * x2) - (x2 * x1) + (x2 * x1) - (x1 * x2) + (x1 * x1) - (x1 * x1) +
           (x2 * x2) - (x2 * x2) + (x1 * x1) - (x1 * x1) + (x2 / x2) + (x2 / x2) - (x1 / x1) + (x1 / x1) - (x1 / x2) +
           (x2 / x1) - (x2 / x1) + (x1 / x2) - (x1 / x2) + (x2 / x1) - (x2 / x1) + (x1 / x2) - (x1 / x2) + (x2 / x1) -
           (x2 / x1) + (x1 / x2) - (x1 / x1) + (x1 / x1) - (x2 / x2) + (x2 / x2) - (x1 / x1) + (x1 / x1) - (x2 / x2) +
           (x2 / x2);
  };

  for (const auto &&_ : state) {
    auto start = Clock::now();
    std::for_each(idx.begin(), idx.end(), [&](const auto &i) {
      out[i].x1 = compute(v1[i].x1, v2[i].x1);
      out[i].x2 = compute(v1[i].x2, v2[i].x2);
      out[i].x3 = compute(v1[i].x3, v2[i].x3);
      out[i].x4 = compute(v1[i].x4, v2[i].x4);
      out[i].x5 = compute(v1[i].x5, v2[i].x5);
      out[i].x6 = compute(v1[i].x6, v2[i].x6);
      out[i].x7 = compute(v1[i].x7, v2[i].x7);
      out[i].x8 = compute(v1[i].x8, v2[i].x8);
    });
    auto end = Clock::now();

    auto elapsed_seconds = std::chrono::duration_cast<fsecs>(end - start);
    state.SetIterationTime(elapsed_seconds.count());
  }

  state.counters["size"] = out.size();
  state.counters["random"] = state.range(0);
  state.counters["stride"] = state.range(1);
}

/*
 * SoA
 */
template <class Container>
static void BM_MemoryBoundSoA(benchmark::State &state) {
  Container data;

  std::vector<size_t> idx;
  if (state.range(0)) {
    idx = generateRandomIndices(data.vout_x1.size());
  } else {
    idx = generateIndicesWithStride(data.vout_x1.size(), 2);
  }

  for (const auto &&_ : state) {
    auto start = Clock::now();
    std::for_each(idx.begin(), idx.end(), [&](const auto &i) {
      data.vout_x1[i] = data.v1_x1[i] + data.v2_x1[i];
      data.vout_x2[i] = data.v1_x2[i] + data.v2_x2[i];
      data.vout_x3[i] = data.v1_x3[i] + data.v2_x3[i];
      data.vout_x4[i] = data.v1_x4[i] + data.v2_x4[i];
      data.vout_x5[i] = data.v1_x5[i] + data.v2_x5[i];
      data.vout_x6[i] = data.v1_x6[i] + data.v2_x6[i];
      data.vout_x7[i] = data.v1_x7[i] + data.v2_x7[i];
      data.vout_x8[i] = data.v1_x8[i] + data.v2_x8[i];
    });
    auto end = Clock::now();

    auto elapsed_seconds = std::chrono::duration_cast<fsecs>(end - start);
    state.SetIterationTime(elapsed_seconds.count());
  }

  state.counters["size"] = data.out.size();
  state.counters["random"] = state.range(0);
  state.counters["stride"] = state.range(1);
}

template <class Container>
static void BM_ComputeBoundSoA(benchmark::State &state) {
  Container data;

  std::vector<size_t> idx;
  if (state.range(0)) {
    idx = generateRandomIndices(data.vout_x1.size());
  } else {
    idx = generateIndicesWithStride(data.vout_x1.size(), state.range(1));
  }

  auto compute = [](const auto &x1, const auto &x2) {
    return (x1 - x2) * (x2 - x1) + (x2 - x1) * (x1 - x2) + (x1 + x2) * (x2 - x1) + (x2 - x1) * (x1 + x2) +
           (x1 + x2) * (x2 + x1) + (x2 + x1) * (x1 + x2) + (x1 - x1) * (x1 - x1) + (x2 - x2) * (x2 - x2) +
           (x1 + x1) * (x1 + x1) + (x2 + x2) * (x2 + x2) + (x1 - x1) * (x1 - x1) - (x1 - x2) / (x2 - x1) -
           (x2 - x1) / (x1 - x2) - (x1 + x2) / (x2 - x1) - (x2 - x1) / (x1 + x2) - (x1 + x2) / (x2 + x1) -
           (x2 + x1) / (x1 + x2) - (x1 - x1) / (x1 - x1) - (x2 - x2) / (x2 - x2) - (x1 + x1) / (x1 + x1) -
           (x2 + x2) / (x2 + x2) - (x1 * x1) - (x1 * x1) + (x1 * x2) - (x2 * x1) + (x2 * x1) - (x1 * x2) + (x1 * x2) -
           (x2 * x1) + (x2 * x1) - (x1 * x2) + (x1 * x2) - (x2 * x1) + (x2 * x1) - (x1 * x2) + (x1 * x1) - (x1 * x1) +
           (x2 * x2) - (x2 * x2) + (x1 * x1) - (x1 * x1) + (x2 / x2) + (x2 / x2) - (x1 / x1) + (x1 / x1) - (x1 / x2) +
           (x2 / x1) - (x2 / x1) + (x1 / x2) - (x1 / x2) + (x2 / x1) - (x2 / x1) + (x1 / x2) - (x1 / x2) + (x2 / x1) -
           (x2 / x1) + (x1 / x2) - (x1 / x1) + (x1 / x1) - (x2 / x2) + (x2 / x2) - (x1 / x1) + (x1 / x1) - (x2 / x2) +
           (x2 / x2);
  };

  for (const auto &&_ : state) {
    auto start = Clock::now();
    std::for_each(idx.begin(), idx.end(), [&](const auto &i) {
      data.vout_x1[i] = compute(data.v1_x1[i], data.v2_x1[i]);
      data.vout_x2[i] = compute(data.v1_x2[i], data.v2_x2[i]);
      data.vout_x3[i] = compute(data.v1_x3[i], data.v2_x3[i]);
      data.vout_x4[i] = compute(data.v1_x4[i], data.v2_x4[i]);
      data.vout_x5[i] = compute(data.v1_x5[i], data.v2_x5[i]);
      data.vout_x6[i] = compute(data.v1_x6[i], data.v2_x6[i]);
      data.vout_x7[i] = compute(data.v1_x7[i], data.v2_x7[i]);
      data.vout_x8[i] = compute(data.v1_x8[i], data.v2_x8[i]);
    });
    auto end = Clock::now();

    auto elapsed_seconds = std::chrono::duration_cast<fsecs>(end - start);
    state.SetIterationTime(elapsed_seconds.count());
  }

  state.counters["size"] = data.out.size();
  state.counters["random"] = state.range(0);
  state.counters["stride"] = state.range(1);
}

#define BENCHMARK_ARGS                                                                                                 \
  Unit(benchmark::kMillisecond)->UseManualTime()->ArgsProduct({{0}, benchmark::CreateRange(1, 8, /*multi=*/2)});

#define BENCHMARK_ALL_TYPES(OP, MEMBERS, TYPE, SIZE)                                                                   \
  BENCHMARK_TEMPLATE(BM_##OP##AoS, AoS##MEMBERS##Vec<TYPE, container_size>)->BENCHMARK_ARGS;                           \
  BENCHMARK_TEMPLATE(BM_##OP##SoA, SoA##MEMBERS##Vec<TYPE, container_size>)->BENCHMARK_ARGS;

#define BENCHMARK_ALL_MEMBERS(TYPE, SIZE)                                                                              \
  BENCHMARK_ALL_TYPES(MemoryBound, 8, TYPE, SIZE)                                                                      \
  BENCHMARK_ALL_TYPES(ComputeBound, 8, TYPE, SIZE)

// BENCHMARK_ALL_MEMBERS(float, container_size)
BENCHMARK_ALL_MEMBERS(double, container_size)

BENCHMARK_MAIN();
