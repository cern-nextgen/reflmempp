#include <benchmark/benchmark.h>
#include <chrono>
#include <iostream>
#include <numeric>
#include <span>
#include <vector>

#include <chrono>
using Clock = std::chrono::steady_clock;
using fsecs = std::chrono::duration<double, std::chrono::seconds::period>;

#define CHECK_CUDA_ERROR(call)                                                                                         \
  do {                                                                                                                 \
    cudaError_t err = call;                                                                                            \
    if (err != cudaSuccess) {                                                                                          \
      fprintf(stderr, "CUDA error in file '%s' in line %i: %s.\n", __FILE__, __LINE__, cudaGetErrorString(err));       \
      exit(EXIT_FAILURE);                                                                                              \
    }                                                                                                                  \
  } while (0)

constexpr size_t container_size = 2 << 23;

#ifdef __CUDACC__
#define __HOST_DEVICE__ __host__ __device__
#else
#define __HOST_DEVICE__
#endif

// #define USE_CUDA_EVENTS

// https://github.com/cern-nextgen/wp1.7-soa-wrapper/blob/0aa71e432691425353ea3c4ba7cd6f51f480746c/allocator.h#L10

enum class layout { aos = 0, soa = 1 };
enum class location { host = 0, device = 1 };

template <typename T, location D>
struct SRef;

template <typename T>
struct SRef<T, location::host> {
  std::span<T> x, y, z;

  struct Ref {
    T &x, &y, &z;
  };

  struct RefConst {
    const T &x, &y, &z;
  };

  constexpr static size_t alignment = 128;

  Ref operator[](size_t i) { return {x[i], y[i], z[i]}; }

  const RefConst operator[](size_t i) const { return {x[i], y[i], z[i]}; }

  // Helper function to compute aligned size
  constexpr inline size_t align_size(size_t size) const { return ((size + alignment - 1) / alignment) * alignment; }

  SRef(size_t n) {
    storage.resize(align_size(3 * n * sizeof(T)));
    new (storage.data()) T[n];

    x = std::span<T>(reinterpret_cast<T *>(storage.data()), n);
    y = std::span<T>(reinterpret_cast<T *>(storage.data() + align_size(n * sizeof(T))), n);
    z = std::span<T>(reinterpret_cast<T *>(storage.data() + align_size(2 * n * sizeof(T))), n);
  }

  void *data() const { return (void *)storage.data(); }
  size_t size() const { return storage.size(); }

private:
  alignas(64) std::vector<std::byte> storage;
};

template <typename T>
struct SRef<T, location::device> {
  std::span<T> x, y, z;

  struct Ref {
    T &x, &y, &z;
  };

  struct RefConst {
    const T &x, &y, &z;
  };

  constexpr static size_t alignment = 128;

  __HOST_DEVICE__ Ref operator[](size_t i) { return {x[i], y[i], z[i]}; }

  __HOST_DEVICE__ const RefConst operator[](size_t i) const { return {x[i], y[i], z[i]}; }

  // Helper function to compute aligned size
  __HOST_DEVICE__ constexpr inline size_t align_size(size_t size) const {
    return ((size + alignment - 1) / alignment) * alignment;
  }

  SRef(size_t n) {
    CHECK_CUDA_ERROR(cudaMalloc(&storage, align_size(3 * n * sizeof(T))));
    x = std::span<T>(reinterpret_cast<T *>(storage), n);
    y = std::span<T>(reinterpret_cast<T *>(storage) + align_size(n * sizeof(T)), n);
    z = std::span<T>(reinterpret_cast<T *>(storage) + align_size(2 * n * sizeof(T)), n);
  }

  void *data() const { return storage; }

private:
  alignas(64) void *storage;
};

template <typename T>
__global__ void MulTwoAoS(SRef<T, location::device> s) {
  auto tid = threadIdx.x + blockIdx.x * blockDim.x;
  if (tid < container_size) { s[tid].x = tid * 2; }
}

template <typename T>
__global__ void MulTwoSoA(SRef<T, location::device> s) {
  auto tid = threadIdx.x + blockIdx.x * blockDim.x;
  if (tid < container_size) { s.x[tid] = tid * 2; }
}

////////////////////////////////////////////////////////////////////

template <typename T>
static void BM_ReferencesAoSGPU(benchmark::State &state) {
  SRef<T, location::host> h_out(container_size);
  SRef<T, location::device> d_in(container_size);

  for (const auto &&_ : state) {
#ifdef USE_CUDA_EVENTS
    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);
    cudaEventRecord(start);
#else
    cudaDeviceSynchronize();
    auto start = Clock::now();
#endif

    MulTwoAoS<<<ceil(container_size / 256.), 256>>>(d_in);

#ifdef USE_CUDA_EVENTS
    cudaEventRecord(stop);
    cudaEventSynchronize(stop);
    float milliseconds = 0;
    cudaEventElapsedTime(&milliseconds, start, stop);
    state.SetIterationTime(milliseconds / 1000.);
#else
    cudaDeviceSynchronize();
    auto end = Clock::now();
    auto elapsed_seconds = std::chrono::duration_cast<fsecs>(end - start);
    state.SetIterationTime(elapsed_seconds.count());
#endif

    CHECK_CUDA_ERROR(cudaGetLastError());
    state.counters["size"] = container_size;
  }

  CHECK_CUDA_ERROR(cudaMemcpy(h_out.data(), d_in.data(), h_out.size(), cudaMemcpyDeviceToHost));
  int i = 0;
  if (!std::all_of(h_out.x.begin(), h_out.x.end(), [&](int val) { return 2 * i++ == val; })) {
    state.SkipWithError("Incorrect result!");
    for (size_t j = 0; j < 10 && j < h_out.x.size(); ++j) {
      std::cout << h_out.x[j] << " ";
    }
    std::cout << std::endl;
  }

  cudaFree(d_in.data());
}

template <typename T>
static void BM_ReferencesSoAGPU(benchmark::State &state) {
  SRef<T, location::host> h_out(container_size);
  SRef<T, location::device> d_in(container_size);

  for (const auto &&_ : state) {
#ifdef USE_CUDA_EVENTS
    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);
    cudaEventRecord(start);
#else
    cudaDeviceSynchronize();
    auto start = Clock::now();
#endif

    MulTwoSoA<<<ceil(container_size / 256.), 256>>>(d_in);

#ifdef USE_CUDA_EVENTS
    cudaEventRecord(stop);
    cudaEventSynchronize(stop);
    float milliseconds = 0;
    cudaEventElapsedTime(&milliseconds, start, stop);
    state.SetIterationTime(milliseconds / 1000.);
#else
    cudaDeviceSynchronize();
    auto end = Clock::now();
    auto elapsed_seconds = std::chrono::duration_cast<fsecs>(end - start);
    state.SetIterationTime(elapsed_seconds.count());
#endif
    CHECK_CUDA_ERROR(cudaGetLastError());

    state.counters["size"] = container_size;
  }

  CHECK_CUDA_ERROR(cudaMemcpy(h_out.data(), d_in.data(), h_out.size(), cudaMemcpyDeviceToHost));
  int i = 0;
  if (!std::all_of(h_out.x.begin(), h_out.x.end(), [&](int val) { return 2 * i++ == val; })) {
    state.SkipWithError("Incorrect result!");
    for (size_t j = 0; j < 10 && j < h_out.x.size(); ++j) {
      std::cout << h_out.x[j] << " ";
    }
    std::cout << std::endl;
  }

  cudaFree(d_in.data());
}

#define BENCHMARK_ARGS Unit(benchmark::kMillisecond)->UseManualTime();

BENCHMARK_TEMPLATE(BM_ReferencesAoSGPU, double)->BENCHMARK_ARGS;
BENCHMARK_TEMPLATE(BM_ReferencesSoAGPU, double)->BENCHMARK_ARGS;

BENCHMARK_MAIN();
