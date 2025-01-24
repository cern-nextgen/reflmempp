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
  auto &v = data.v1;
  auto &out = data.out;

  for (const auto &&_ : state) {
    auto start = Clock::now();
    std::transform(v.begin(), v.end(), out.begin(), [](const auto &in) {
      return (in.x1 - in.x2) * (in.x1 - in.x2) + (in.x2 - in.x1) * (in.x2 - in.x1);
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
  auto &v = data.v1;
  auto &out = data.out;

  for (const auto &&_ : state) {
    auto start = Clock::now();
    std::transform(v.begin(), v.end(), out.begin(), [](const auto &in) {
      return (in.x1 - in.x2) * (in.x2 - in.x1) + (in.x2 - in.x1) * (in.x1 - in.x2) + (in.x1 + in.x2) * (in.x2 - in.x1) +
             (in.x2 - in.x1) * (in.x1 + in.x2) + (in.x1 + in.x2) * (in.x2 + in.x1) + (in.x2 + in.x1) * (in.x1 + in.x2) +
             (in.x1 - in.x1) * (in.x1 - in.x1) + (in.x2 - in.x2) * (in.x2 - in.x2) + (in.x1 + in.x1) * (in.x1 + in.x1) +
             (in.x2 + in.x2) * (in.x2 + in.x2) + (in.x1 - in.x1) * (in.x1 - in.x1) - (in.x1 - in.x2) / (in.x2 - in.x1) -
             (in.x2 - in.x1) / (in.x1 - in.x2) - (in.x1 + in.x2) / (in.x2 - in.x1) - (in.x2 - in.x1) / (in.x1 + in.x2) -
             (in.x1 + in.x2) / (in.x2 + in.x1) - (in.x2 + in.x1) / (in.x1 + in.x2) - (in.x1 - in.x1) / (in.x1 - in.x1) -
             (in.x2 - in.x2) / (in.x2 - in.x2) - (in.x1 + in.x1) / (in.x1 + in.x1) - (in.x2 + in.x2) / (in.x2 + in.x2) -
             (in.x1 * in.x1) - (in.x1 * in.x1) + (in.x1 * in.x2) - (in.x2 * in.x1) + (in.x2 * in.x1) - (in.x1 * in.x2) +
             (in.x1 * in.x2) - (in.x2 * in.x1) + (in.x2 * in.x1) - (in.x1 * in.x2) + (in.x1 * in.x2) - (in.x2 * in.x1) +
             (in.x2 * in.x1) - (in.x1 * in.x2) + (in.x1 * in.x1) - (in.x1 * in.x1) + (in.x2 * in.x2) - (in.x2 * in.x2) +
             (in.x1 * in.x1) - (in.x1 * in.x1) + (in.x2 / in.x2) + (in.x2 / in.x2) - (in.x1 / in.x1) + (in.x1 / in.x1) -
             (in.x1 / in.x2) + (in.x2 / in.x1) - (in.x2 / in.x1) + (in.x1 / in.x2) - (in.x1 / in.x2) + (in.x2 / in.x1) -
             (in.x2 / in.x1) + (in.x1 / in.x2) - (in.x1 / in.x2) + (in.x2 / in.x1) - (in.x2 / in.x1) + (in.x1 / in.x2) -
             (in.x1 / in.x1) + (in.x1 / in.x1) - (in.x2 / in.x2) + (in.x2 / in.x2) - (in.x1 / in.x1) + (in.x1 / in.x1) -
             (in.x2 / in.x2) + (in.x2 / in.x2);
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
  auto deltaR2 = [](const auto &x1, const auto &x2) { return (x1 - x2) * (x1 - x2) + (x2 - x1) * (x2 - x1); };

  auto &v_x1 = data.v1_x1;
  auto &v_x2 = data.v1_x2;
  auto &out = data.out;

  for (const auto &&_ : state) {
    auto start = Clock::now();
    std::ranges::copy(std::views::zip_transform(deltaR2, v_x1, v_x2), out.begin());
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
  auto deltaR2 = [](const auto &x1, const auto &x2) {
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

  auto &v_x1 = data.v1_x1;
  auto &v_x2 = data.v1_x2;
  auto &out = data.out;

  for (const auto &&_ : state) {
    auto start = Clock::now();
    std::ranges::copy(std::views::zip_transform(deltaR2, v_x1, v_x2), out.begin());
    auto end = Clock::now();

    auto elapsed_seconds = std::chrono::duration_cast<fsecs>(end - start);
    state.SetIterationTime(elapsed_seconds.count());
  }

  state.counters["size"] = data.out.size();
}

#define BENCHMARK_ARGS Unit(benchmark::kMillisecond)->UseManualTime();

#define BENCHMARK_ALL_TYPES(OP, MEMBERS, TYPE, SIZE)                                                                   \
  BENCHMARK_TEMPLATE(BM_##OP##AoS, AoS##MEMBERS##Vec<TYPE, container_size>)->BENCHMARK_ARGS;                           \
  BENCHMARK_TEMPLATE(BM_##OP##SoA, SoA##MEMBERS##Vec<TYPE, container_size>)->BENCHMARK_ARGS;
  // BENCHMARK_TEMPLATE(BM_##OP##AoS, AoS##MEMBERS##Raw<TYPE, SIZE>)->BENCHMARK_ARGS;                                     \
  // BENCHMARK_TEMPLATE(BM_##OP##SoA, SoA##MEMBERS##Raw<TYPE, SIZE>)->BENCHMARK_ARGS;                                     \
  // BENCHMARK_TEMPLATE(BM_##OP##AoS, AoS##MEMBERS##Arr<TYPE, container_size>)->BENCHMARK_ARGS;                           \
  // BENCHMARK_TEMPLATE(BM_##OP##SoA, SoA##MEMBERS##Arr<TYPE, container_size>)->BENCHMARK_ARGS;

#define BENCHMARK_ALL_MEMBERS(TYPE, SIZE)                                                                              \
  BENCHMARK_ALL_TYPES(MemoryBound, 2, TYPE, SIZE)                                                                      \
  BENCHMARK_ALL_TYPES(MemoryBound, 4, TYPE, SIZE)                                                                      \
  BENCHMARK_ALL_TYPES(MemoryBound, 8, TYPE, SIZE)                                                                      \
  BENCHMARK_ALL_TYPES(MemoryBound, 20, TYPE, SIZE)                                                                     \
  BENCHMARK_ALL_TYPES(ComputeBound, 2, TYPE, SIZE)                                                                     \
  BENCHMARK_ALL_TYPES(ComputeBound, 4, TYPE, SIZE)                                                                     \
  BENCHMARK_ALL_TYPES(ComputeBound, 8, TYPE, SIZE)                                                                     \
  BENCHMARK_ALL_TYPES(ComputeBound, 20, TYPE, SIZE)

// BENCHMARK_ALL_MEMBERS(float, container_size)
BENCHMARK_ALL_MEMBERS(double, container_size)

BENCHMARK_MAIN();
