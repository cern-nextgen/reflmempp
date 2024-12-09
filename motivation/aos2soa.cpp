#include "aos.h"
#include "soa.h"
#include <algorithm>
#include <array>
#include <benchmark/benchmark.h>
#include <cmath>
#include <iostream>
#include <ranges>
#include <vector>

#include <chrono>
using Clock = std::chrono::steady_clock;
using fsecs = std::chrono::duration<double, std::chrono::seconds::period>;

constexpr size_t container_size = 10e6;

/*
 * AoS
 */

template <typename Container>
static void BM_MemoryBoundAoS(benchmark::State &state) {
  Container data;
  auto &v1 = data.v1;
  auto &v2 = data.v2;
  auto &out = data.out;

  for (const auto &&_ : state) {
    auto start = Clock::now();
    std::transform(v1.begin(), v1.end(), v2.begin(), out.begin(), [](const auto &s1, const auto &s2) {
      return (s2.x1 - s1.x1) * (s2.x1 - s1.x1) + (s2.x2 - s1.x2) * (s2.x2 - s1.x2);
    });
    auto end = Clock::now();

    auto elapsed_seconds = std::chrono::duration_cast<fsecs>(end - start);
    state.SetIterationTime(elapsed_seconds.count());

    if (out.size() == 0 || !std::all_of(out.begin(), out.end(), [](int i) { return i == 2; })) {
      state.SkipWithError("Incorrect result!");
    }
  }

  state.counters["size"] = out.size();
}

template <typename Container>
static void BM_ComputeBoundAoS(benchmark::State &state) {
  Container data;
  auto &v1 = data.v1;
  auto &v2 = data.v2;
  auto &out = data.out;

  for (const auto &&_ : state) {
    auto start = Clock::now();
    std::transform(v1.begin(), v1.end(), v2.begin(), out.begin(), [](const auto &s1, const auto &s2) {
      return (s2.x1 - s1.x2) * (s2.x2 - s1.x1) + (s2.x2 - s1.x1) * (s2.x1 - s1.x2) + (s2.x1 + s1.x2) * (s2.x2 - s1.x1) +
             (s2.x2 - s1.x1) * (s2.x1 + s1.x2) + (s2.x1 + s1.x2) * (s2.x2 + s1.x1) + (s2.x2 + s1.x1) * (s2.x1 + s1.x2) +
             (s2.x1 - s1.x1) * (s2.x1 - s1.x1) + (s2.x2 - s1.x2) * (s2.x2 - s1.x2) + (s2.x1 + s1.x1) * (s2.x1 + s1.x1) +
             (s2.x2 + s1.x2) * (s2.x2 + s1.x2) + (s2.x1 - s1.x1) * (s2.x1 - s1.x1) - (s2.x1 - s1.x2) / (s2.x2 - s1.x1) -
             (s2.x2 - s1.x1) / (s2.x1 - s1.x2) - (s2.x1 + s1.x2) / (s2.x2 - s1.x1) - (s2.x2 - s1.x1) / (s2.x1 + s1.x2) -
             (s2.x1 + s1.x2) / (s2.x2 + s1.x1) - (s2.x2 + s1.x1) / (s2.x1 + s1.x2) - (s2.x1 - s1.x1) / (s2.x1 - s1.x1) -
             (s2.x2 - s1.x2) / (s2.x2 - s1.x2) - (s2.x1 + s1.x1) / (s2.x1 + s1.x1) - (s2.x2 + s1.x2) / (s2.x2 + s1.x2) -
             (s2.x1 * s1.x1) - (s2.x1 * s1.x1) + (s2.x1 * s1.x2) - (s2.x2 * s1.x1) + (s2.x2 * s1.x1) - (s2.x1 * s1.x2) +
             (s2.x1 * s1.x2) - (s2.x2 * s1.x1) + (s2.x2 * s1.x1) - (s2.x1 * s1.x2) + (s2.x1 * s1.x2) - (s2.x2 * s1.x1) +
             (s2.x2 * s1.x1) - (s2.x1 * s1.x2) + (s2.x1 * s1.x1) - (s2.x1 * s1.x1) + (s2.x2 * s1.x2) - (s2.x2 * s1.x2) +
             (s2.x1 * s1.x1) - (s2.x1 * s1.x1) + (s2.x2 / s1.x2) + (s2.x2 / s1.x2) - (s2.x1 / s1.x1) + (s2.x1 / s1.x1) -
             (s2.x1 / s1.x2) + (s2.x2 / s1.x1) - (s2.x2 / s1.x1) + (s2.x1 / s1.x2) - (s2.x1 / s1.x2) + (s2.x2 / s1.x1) -
             (s2.x2 / s1.x1) + (s2.x1 / s1.x2) - (s2.x1 / s1.x2) + (s2.x2 / s1.x1) - (s2.x2 / s1.x1) + (s2.x1 / s1.x2) -
             (s2.x1 / s1.x1) + (s2.x1 / s1.x1) - (s2.x2 / s1.x2) + (s2.x2 / s1.x2) - (s2.x1 / s1.x1) + (s2.x1 / s1.x1) -
             (s2.x2 / s1.x2) + (s2.x2 / s1.x2);
    });
    auto end = Clock::now();

    auto elapsed_seconds = std::chrono::duration_cast<fsecs>(end - start);
    state.SetIterationTime(elapsed_seconds.count());
  }

  state.counters["size"] = out.size();
}

/*
 * SoA
 */
template <class Container>
static void BM_MemoryBoundSoA(benchmark::State &state) {
  Container data;
  auto deltaR2 = [](const auto &v1_x1, const auto &v2_x1, const auto &v1_x2, const auto &v2_x2) {
    return (v2_x1 - v1_x1) * (v2_x1 - v1_x1) + (v2_x2 - v1_x2) * (v2_x2 - v1_x2);
  };

  auto &v1_x1 = data.v1_x1;
  auto &v2_x1 = data.v2_x1;
  auto &v1_x2 = data.v1_x2;
  auto &v2_x2 = data.v2_x2;
  auto &out = data.out;

  for (const auto &&_ : state) {
    auto start = Clock::now();
    std::ranges::copy(std::views::zip_transform(deltaR2, v1_x1, v2_x1, v1_x2, v2_x2), out.begin());
    auto end = Clock::now();

    auto elapsed_seconds = std::chrono::duration_cast<fsecs>(end - start);
    state.SetIterationTime(elapsed_seconds.count());

    if (out.size() == 0 || !std::all_of(data.out.begin(), data.out.end(), [](int i) { return i == 2; })) {
      state.SkipWithError("Incorrect result!");
    }
  }

  state.counters["size"] = data.out.size();
}

template <class Container>
static void BM_ComputeBoundSoA(benchmark::State &state) {
  Container data;
  auto deltaR2 = [](const auto &v1_x1, const auto &v2_x1, const auto &v1_x2, const auto &v2_x2) {
    return (v2_x1 - v1_x2) * (v2_x2 - v1_x1) + (v2_x2 - v1_x1) * (v2_x1 - v1_x2) + (v2_x1 + v1_x2) * (v2_x2 - v1_x1) +
           (v2_x2 - v1_x1) * (v2_x1 + v1_x2) + (v2_x1 + v1_x2) * (v2_x2 + v1_x1) + (v2_x2 + v1_x1) * (v2_x1 + v1_x2) +
           (v2_x1 - v1_x1) * (v2_x1 - v1_x1) + (v2_x2 - v1_x2) * (v2_x2 - v1_x2) + (v2_x1 + v1_x1) * (v2_x1 + v1_x1) +
           (v2_x2 + v1_x2) * (v2_x2 + v1_x2) + (v2_x1 - v1_x1) * (v2_x1 - v1_x1) - (v2_x1 - v1_x2) / (v2_x2 - v1_x1) -
           (v2_x2 - v1_x1) / (v2_x1 - v1_x2) - (v2_x1 + v1_x2) / (v2_x2 - v1_x1) - (v2_x2 - v1_x1) / (v2_x1 + v1_x2) -
           (v2_x1 + v1_x2) / (v2_x2 + v1_x1) - (v2_x2 + v1_x1) / (v2_x1 + v1_x2) - (v2_x1 - v1_x1) / (v2_x1 - v1_x1) -
           (v2_x2 - v1_x2) / (v2_x2 - v1_x2) - (v2_x1 + v1_x1) / (v2_x1 + v1_x1) - (v2_x2 + v1_x2) / (v2_x2 + v1_x2) -
           (v2_x1 * v1_x1) - (v2_x1 * v1_x1) + (v2_x1 * v1_x2) - (v2_x2 * v1_x1) + (v2_x2 * v1_x1) - (v2_x1 * v1_x2) +
           (v2_x1 * v1_x2) - (v2_x2 * v1_x1) + (v2_x2 * v1_x1) - (v2_x1 * v1_x2) + (v2_x1 * v1_x2) - (v2_x2 * v1_x1) +
           (v2_x2 * v1_x1) - (v2_x1 * v1_x2) + (v2_x1 * v1_x1) - (v2_x1 * v1_x1) + (v2_x2 * v1_x2) - (v2_x2 * v1_x2) +
           (v2_x1 * v1_x1) - (v2_x1 * v1_x1) + (v2_x2 / v1_x2) + (v2_x2 / v1_x2) - (v2_x1 / v1_x1) + (v2_x1 / v1_x1) -
           (v2_x1 / v1_x2) + (v2_x2 / v1_x1) - (v2_x2 / v1_x1) + (v2_x1 / v1_x2) - (v2_x1 / v1_x2) + (v2_x2 / v1_x1) -
           (v2_x2 / v1_x1) + (v2_x1 / v1_x2) - (v2_x1 / v1_x2) + (v2_x2 / v1_x1) - (v2_x2 / v1_x1) + (v2_x1 / v1_x2) -
           (v2_x1 / v1_x1) + (v2_x1 / v1_x1) - (v2_x2 / v1_x2) + (v2_x2 / v1_x2) - (v2_x1 / v1_x1) + (v2_x1 / v1_x1) -
           (v2_x2 / v1_x2) + (v2_x2 / v1_x2);
  };

  auto &v1_x1 = data.v1_x1;
  auto &v2_x1 = data.v2_x1;
  auto &v1_x2 = data.v1_x2;
  auto &v2_x2 = data.v2_x2;
  auto &out = data.out;

  for (const auto &&_ : state) {
    auto start = Clock::now();
    std::ranges::copy(std::views::zip_transform(deltaR2, v1_x1, v2_x1, v1_x2, v2_x2), out.begin());
    auto end = Clock::now();

    auto elapsed_seconds = std::chrono::duration_cast<fsecs>(end - start);
    state.SetIterationTime(elapsed_seconds.count());
  }

  state.counters["size"] = data.out.size();
}

#define BENCHMARK_ALL_TYPES(OP, MEMBERS, TYPE, SIZE)                                                                   \
  BENCHMARK_TEMPLATE(BM_##OP##AoS, AoS##MEMBERS##Raw<TYPE, SIZE>)->Unit(benchmark::kMillisecond)->UseManualTime();     \
  BENCHMARK_TEMPLATE(BM_##OP##SoA, SoA##MEMBERS##Raw<TYPE, SIZE>)->Unit(benchmark::kMillisecond)->UseManualTime();     \
  BENCHMARK_TEMPLATE(BM_##OP##AoS, AoS##MEMBERS##Arr<TYPE, container_size>)                                            \
      ->Unit(benchmark::kMillisecond)                                                                                  \
      ->UseManualTime();                                                                                               \
  BENCHMARK_TEMPLATE(BM_##OP##SoA, SoA##MEMBERS##Arr<TYPE, container_size>)                                            \
      ->Unit(benchmark::kMillisecond)                                                                                  \
      ->UseManualTime();                                                                                               \
  BENCHMARK_TEMPLATE(BM_##OP##AoS, AoS##MEMBERS##Vec<TYPE, container_size>)                                            \
      ->Unit(benchmark::kMillisecond)                                                                                  \
      ->UseManualTime();                                                                                               \
  BENCHMARK_TEMPLATE(BM_##OP##SoA, SoA##MEMBERS##Vec<TYPE, container_size>)                                            \
      ->Unit(benchmark::kMillisecond)                                                                                  \
      ->UseManualTime();

#define BENCHMARK_ALL_MEMBERS(TYPE, SIZE)                                                                              \
  BENCHMARK_ALL_TYPES(MemoryBound, 2, TYPE, SIZE)                                                                      \
  BENCHMARK_ALL_TYPES(MemoryBound, 4, TYPE, SIZE)                                                                      \
  BENCHMARK_ALL_TYPES(MemoryBound, 8, TYPE, SIZE)                                                                      \
  BENCHMARK_ALL_TYPES(MemoryBound, 20, TYPE, SIZE)                                                                     \
  BENCHMARK_ALL_TYPES(ComputeBound, 2, TYPE, SIZE)                                                                     \
  BENCHMARK_ALL_TYPES(ComputeBound, 4, TYPE, SIZE)                                                                     \
  BENCHMARK_ALL_TYPES(ComputeBound, 8, TYPE, SIZE)                                                                     \
  BENCHMARK_ALL_TYPES(ComputeBound, 20, TYPE, SIZE)

BENCHMARK_ALL_MEMBERS(float, container_size)
BENCHMARK_ALL_MEMBERS(double, container_size)

BENCHMARK_MAIN();
