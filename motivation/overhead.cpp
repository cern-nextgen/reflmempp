#include <benchmark/benchmark.h>
#include <chrono>
#include <numeric>
#include <vector>

#include <chrono>
using Clock = std::chrono::steady_clock;
using fsecs = std::chrono::duration<double, std::chrono::seconds::period>;

constexpr size_t container_size = 10e6;

template <typename T>
struct SRef {
  std::span<T> x, y, z;

  struct Ref {
    T &x, &y, &z;
  };

  struct RefConst {
    const T &x, &y, &z;
  };

  Ref operator[](size_t i) { return {
    x[i], y[i], z[i]};
  }

  const RefConst operator[](size_t i) const { return {x[i], y[i], z[i]}; }

  // Helper function to compute aligned size
  constexpr inline size_t align_size(size_t size, size_t alignment) const {
    return ((size + alignment - 1) / alignment) * alignment;
  }

  SRef(size_t n) {
    storage.resize(align_size(3 * n * sizeof(T), 64));
    new (storage.data()) T[n];

    x = std::span<T>(reinterpret_cast<T *>(storage.data()), n);
    y = std::span<T>(reinterpret_cast<T *>(storage.data() + align_size(n * sizeof(T), 64)), n);
    z = std::span<T>(reinterpret_cast<T *>(storage.data() + align_size(2 * n * sizeof(T), 64)), n);
    std::iota(x.begin(), x.end(), 0.0);
    std::iota(y.begin(), y.end(), 0.0);
    std::iota(z.begin(), z.end(), 0.0);
  }

private:
  alignas(64) std::vector<std::byte> storage;
};

template <typename T>
static void BM_ReferencesAoS(benchmark::State &state) {
  SRef<T> s(container_size);

  for (const auto &&_ : state) {
    auto start = Clock::now();
    for (size_t i = 0; i < container_size; i++) {
      s[i].x *= 2;
    }
    auto end = Clock::now();

    auto elapsed_seconds = std::chrono::duration_cast<fsecs>(end - start);
    state.SetIterationTime(elapsed_seconds.count());
  }
}

template <typename T>
static void BM_ReferencesSoA(benchmark::State &state) {
  SRef<T> s(container_size);

  for (const auto &&_ : state) {
    auto start = Clock::now();
    std::ranges::for_each(s.x, [](double &val) { val *= 2; });
    auto end = Clock::now();

    auto elapsed_seconds = std::chrono::duration_cast<fsecs>(end - start);
    state.SetIterationTime(elapsed_seconds.count());
  }
}

///////////////////////////////////////////////////////////////////////////////////

template <typename T>
struct S {
  std::vector<T> x, y, z;

  S(size_t n) {
    x.resize(n);
    y.resize(n);
    z.resize(n);
    std::iota(x.begin(), x.end(), 0.0);
    std::iota(y.begin(), y.end(), 0.0);
    std::iota(z.begin(), z.end(), 0.0);
  }
};

template <typename T>
static void BM_Values(benchmark::State &state) {
  S<T> s(container_size);

  for (const auto &&_ : state) {
    auto start = Clock::now();
    std::ranges::for_each(s.x, [](double &val) { val *= 2; });
    auto end = Clock::now();

    auto elapsed_seconds = std::chrono::duration_cast<fsecs>(end - start);
    state.SetIterationTime(elapsed_seconds.count());
  }
}

#define BENCHMARK_ARGS Unit(benchmark::kMillisecond)->UseManualTime();
BENCHMARK_TEMPLATE(BM_ReferencesAoS, double)->BENCHMARK_ARGS;
BENCHMARK_TEMPLATE(BM_ReferencesSoA, double)->BENCHMARK_ARGS;
BENCHMARK_TEMPLATE(BM_Values, double)->BENCHMARK_ARGS;

BENCHMARK_MAIN();
