#include "aos.h"
#include "aosoa.h"

#include <barrier>
#include <benchmark/benchmark.h>
#include <chrono>
#include <latch>
#include <thread>
#include <vector>
// #include <likwid-marker.h>

using Clock = std::chrono::steady_clock;
using fsecs = std::chrono::duration<double, std::chrono::seconds::period>;

constexpr size_t container_size = 128;
constexpr size_t repetitions = 10e6;

/*
 * AoS
 */

// 1 Thread
template <typename Container>
static void BM_FalseSharing1AoS(benchmark::State &state) {
  Container data;
  auto &out = data.vout;
  constexpr auto num_threads = 1;

  std::barrier start_point{num_threads + 1}; // + main
  std::barrier end_point{num_threads + 1};   // + main
  auto t1 = [&]() {
    start_point.arrive_and_wait();
    for (size_t _ = 0; _ < repetitions; ++_) {
      for (size_t i = 0; i < out.size(); ++i) {
        out[i].x1 = i;
        out[i].x2 = i;
        out[i].x3 = i;
        out[i].x4 = i;
        out[i].x5 = i;
        out[i].x6 = i;
        out[i].x7 = i;
        out[i].x8 = i;
      };
    }
    end_point.arrive_and_wait();
  };

  for (const auto &&_ : state) {
    // IMPROVEMENT: Reuse threads?
    auto thread = std::jthread(t1);
    start_point.arrive_and_wait();
    auto start = Clock::now();
    end_point.arrive_and_wait();
    auto end = Clock::now();

    auto elapsed_seconds = std::chrono::duration_cast<fsecs>(end - start);
    state.SetIterationTime(elapsed_seconds.count());
  }

  state.counters["size"] = out.size();
  state.counters["num_threads"] = num_threads;
}

// 2 Threads
template <typename Container>
static void BM_FalseSharing2AoS(benchmark::State &state) {
  Container data;
  auto &out = data.vout;
  constexpr auto num_threads = 2;

  std::barrier start_point{num_threads + 1}; // + main
  std::barrier end_point{num_threads + 1};   // + main
  auto t1 = [&]() {
    start_point.arrive_and_wait();
    for (size_t _ = 0; _ < repetitions; ++_) {
      for (size_t i = 0; i < out.size(); ++i) {
        out[i].x1 = i;
        out[i].x2 = i;
        out[i].x3 = i;
        out[i].x4 = i;
      }
    }
    end_point.arrive_and_wait();
  };

  auto t2 = [&]() {
    start_point.arrive_and_wait();
    for (size_t _ = 0; _ < repetitions; ++_) {
      for (size_t i = 0; i < out.size(); ++i) {
        out[i].x5 = i;
        out[i].x6 = i;
        out[i].x7 = i;
        out[i].x8 = i;
      }
    }
    end_point.arrive_and_wait();
  };

  for (const auto &&_ : state) {
    std::vector<std::jthread> threads;
    threads.push_back(std::jthread(t1));
    threads.push_back(std::jthread(t2));

    start_point.arrive_and_wait();
    auto start = Clock::now();
    end_point.arrive_and_wait();
    auto end = Clock::now();

    for (auto &t : threads) {
      t.request_stop();
    }

    auto elapsed_seconds = std::chrono::duration_cast<fsecs>(end - start);
    state.SetIterationTime(elapsed_seconds.count());
  }

  state.counters["size"] = out.size();
  state.counters["num_threads"] = num_threads;
}

// 4 Threads
template <typename Container>
static void BM_FalseSharing4AoS(benchmark::State &state) {
  Container data;
  auto &out = data.vout;
  constexpr auto num_threads = 4;

  std::barrier start_point{num_threads + 1}; // + main
  std::barrier end_point{num_threads + 1};   // + main

  auto t1 = [&]() {
    start_point.arrive_and_wait();
    for (size_t _ = 0; _ < repetitions; ++_) {
      for (size_t i = 0; i < out.size(); ++i) {
        out[i].x1 = i;
        out[i].x2 = i;
      }
    }
    end_point.arrive_and_wait();
  };

  auto t2 = [&]() {
    start_point.arrive_and_wait();
    for (size_t _ = 0; _ < repetitions; ++_) {
      for (size_t i = 0; i < out.size(); ++i) {
        out[i].x3 = i;
        out[i].x4 = i;
      }
    }
    end_point.arrive_and_wait();
  };

  auto t3 = [&]() {
    start_point.arrive_and_wait();
    for (size_t _ = 0; _ < repetitions; ++_) {
      for (size_t i = 0; i < out.size(); ++i) {
        out[i].x5 = i;
        out[i].x6 = i;
      }
    }
    end_point.arrive_and_wait();
  };

  auto t4 = [&]() {
    start_point.arrive_and_wait();
    for (size_t _ = 0; _ < repetitions; ++_) {
      for (size_t i = 0; i < out.size(); ++i) {
        out[i].x7 = i;
        out[i].x8 = i;
      }
    }
    end_point.arrive_and_wait();
  };

  for (const auto &&_ : state) {
    std::vector<std::jthread> threads;
    threads.push_back(std::jthread(t1));
    threads.push_back(std::jthread(t2));
    threads.push_back(std::jthread(t3));
    threads.push_back(std::jthread(t4));

    start_point.arrive_and_wait();
    auto start = Clock::now();
    end_point.arrive_and_wait();
    auto end = Clock::now();

    for (auto &t : threads) {
      t.request_stop();
    }

    auto elapsed_seconds = std::chrono::duration_cast<fsecs>(end - start);
    state.SetIterationTime(elapsed_seconds.count());
  }

  state.counters["size"] = out.size();
  state.counters["num_threads"] = num_threads;
}

// 8 Threads
template <typename Container>
static void BM_FalseSharing8AoS(benchmark::State &state) {
  Container data;
  auto &out = data.vout;
  constexpr auto num_threads = 8;

  std::barrier start_point{num_threads + 1}; // + main
  std::barrier end_point{num_threads + 1};   // + main
  auto t1 = [&]() {
    start_point.arrive_and_wait();
    for (size_t _ = 0; _ < repetitions; ++_) {
      for (size_t i = 0; i < out.size(); ++i) {
        out[i].x1 = i;
      }
    }
    end_point.arrive_and_wait();
  };

  auto t2 = [&]() {
    start_point.arrive_and_wait();
    for (size_t _ = 0; _ < repetitions; ++_) {
      for (size_t i = 0; i < out.size(); ++i) {
        out[i].x2 = i;
      }
    }
    end_point.arrive_and_wait();
  };

  auto t3 = [&]() {
    start_point.arrive_and_wait();
    for (size_t _ = 0; _ < repetitions; ++_) {
      for (size_t i = 0; i < out.size(); ++i) {
        out[i].x3 = i;
      }
    }
    end_point.arrive_and_wait();
  };

  auto t4 = [&]() {
    start_point.arrive_and_wait();
    for (size_t _ = 0; _ < repetitions; ++_) {
      for (size_t i = 0; i < out.size(); ++i) {
        out[i].x4 = i;
      }
    }
    end_point.arrive_and_wait();
  };

  auto t5 = [&]() {
    start_point.arrive_and_wait();
    for (size_t _ = 0; _ < repetitions; ++_) {
      for (size_t i = 0; i < out.size(); ++i) {
        out[i].x5 = i;
      }
    }
    end_point.arrive_and_wait();
  };

  auto t6 = [&]() {
    start_point.arrive_and_wait();
    for (size_t _ = 0; _ < repetitions; ++_) {
      for (size_t i = 0; i < out.size(); ++i) {
        out[i].x6 = i;
      }
    }
    end_point.arrive_and_wait();
  };

  auto t7 = [&]() {
    start_point.arrive_and_wait();
    for (size_t _ = 0; _ < repetitions; ++_) {
      for (size_t i = 0; i < out.size(); ++i) {
        out[i].x7 = i;
      }
    }
    end_point.arrive_and_wait();
  };

  auto t8 = [&]() {
    start_point.arrive_and_wait();
    for (size_t _ = 0; _ < repetitions; ++_) {
      for (size_t i = 0; i < out.size(); ++i) {
        out[i].x8 = i;
      }
    }
    end_point.arrive_and_wait();
  };

  for (const auto &&_ : state) {
    std::vector<std::jthread> threads;
    threads.push_back(std::jthread(t1));
    threads.push_back(std::jthread(t2));
    threads.push_back(std::jthread(t3));
    threads.push_back(std::jthread(t4));
    threads.push_back(std::jthread(t5));
    threads.push_back(std::jthread(t6));
    threads.push_back(std::jthread(t7));
    threads.push_back(std::jthread(t8));

    start_point.arrive_and_wait();
    auto start = Clock::now();
    end_point.arrive_and_wait();
    auto end = Clock::now();

    for (auto &t : threads) {
      t.request_stop();
    }

    auto elapsed_seconds = std::chrono::duration_cast<fsecs>(end - start);
    state.SetIterationTime(elapsed_seconds.count());
  }

  state.counters["size"] = out.size();
  state.counters["num_threads"] = num_threads;
}

/*
 * SoA
 */

// 1 Thread
template <typename Container>
void BM_FalseSharing1SoA(benchmark::State &state) {
  Container data;
  constexpr auto num_threads = 1;

  std::barrier start_point{num_threads + 1}; // + main
  std::barrier end_point{num_threads + 1};   // + main

  auto t1 = [&]() {
    start_point.arrive_and_wait();
    for (size_t _ = 0; _ < repetitions; ++_) {
      for (size_t i = 0; i < data.vout_x1.size(); i++) {
        data.vout_x1[i] = i;
        data.vout_x2[i] = i;
        data.vout_x3[i] = i;
        data.vout_x4[i] = i;
        data.vout_x5[i] = i;
        data.vout_x6[i] = i;
        data.vout_x7[i] = i;
        data.vout_x8[i] = i;
      }
    }
    end_point.arrive_and_wait();
  };

  for (const auto &&_ : state) {
    auto thread = std::jthread(t1);

    start_point.arrive_and_wait();
    auto start = Clock::now();
    end_point.arrive_and_wait();
    auto end = Clock::now();

    thread.request_stop();

    auto elapsed_seconds = std::chrono::duration_cast<fsecs>(end - start);
    state.SetIterationTime(elapsed_seconds.count());
  }

  state.counters["size"] = container_size;
  state.counters["num_threads"] = num_threads;
}

// // 2 Threads
template <typename Container>
void BM_FalseSharing2SoA(benchmark::State &state) {
  Container data;
  constexpr auto num_threads = 2;

  std::barrier start_point{num_threads + 1}; // + main
  std::barrier end_point{num_threads + 1};   // + main
  auto t1 = [&]() {
    start_point.arrive_and_wait();
    for (size_t _ = 0; _ < repetitions; ++_) {
      for (size_t i = 0; i < data.vout_x1.size(); i++) {
        data.vout_x1[i] = i;
        data.vout_x2[i] = i;
        data.vout_x3[i] = i;
        data.vout_x4[i] = i;
      }
    }
    end_point.arrive_and_wait();
  };

  auto t2 = [&]() {
    start_point.arrive_and_wait();
    for (size_t _ = 0; _ < repetitions; ++_) {
      for (size_t i = 0; i < data.vout_x1.size(); i++) {
        data.vout_x5[i] = i;
        data.vout_x6[i] = i;
        data.vout_x7[i] = i;
        data.vout_x8[i] = i;
      }
    }
    end_point.arrive_and_wait();
  };

  for (const auto &&_ : state) {
    std::vector<std::jthread> threads;
    threads.push_back(std::jthread(t1));
    threads.push_back(std::jthread(t2));

    start_point.arrive_and_wait();
    auto start = Clock::now();
    end_point.arrive_and_wait();
    auto end = Clock::now();

    for (auto &t : threads) {
      t.request_stop();
    }

    auto elapsed_seconds = std::chrono::duration_cast<fsecs>(end - start);
    state.SetIterationTime(elapsed_seconds.count());
  }

  state.counters["size"] = container_size;
  state.counters["num_threads"] = num_threads;
}

//4 Threads
template <typename Container>
void BM_FalseSharing4SoA(benchmark::State &state) {
  Container data;
  constexpr auto num_threads = 4;

  std::barrier start_point{num_threads + 1}; // + main
  std::barrier end_point{num_threads + 1};   // + main
  auto t1 = [&]() {
    start_point.arrive_and_wait();
    for (size_t _ = 0; _ < repetitions; ++_) {
      for (size_t i = 0; i < data.vout_x1.size(); i++) {
        data.vout_x1[i] = i;
        data.vout_x2[i] = i;
      }
    }
    end_point.arrive_and_wait();
  };

  auto t2 = [&]() {
    start_point.arrive_and_wait();
    for (size_t _ = 0; _ < repetitions; ++_) {
      for (size_t i = 0; i < data.vout_x1.size(); i++) {
        data.vout_x3[i] = i;
        data.vout_x4[i] = i;
      }
    }
    end_point.arrive_and_wait();
  };

  auto t3 = [&]() {
    start_point.arrive_and_wait();
    for (size_t _ = 0; _ < repetitions; ++_) {
      for (size_t i = 0; i < data.vout_x1.size(); i++) {
        data.vout_x5[i] = i;
        data.vout_x6[i] = i;
      }
    }
    end_point.arrive_and_wait();
  };

  auto t4 = [&]() {
    start_point.arrive_and_wait();
    for (size_t _ = 0; _ < repetitions; ++_) {
      for (size_t i = 0; i < data.vout_x1.size(); i++) {
        data.vout_x7[i] = i;
        data.vout_x8[i] = i;
      }
    }
    end_point.arrive_and_wait();
  };

  for (const auto &&_ : state) {
    std::vector<std::jthread> threads;
    threads.push_back(std::jthread(t1));
    threads.push_back(std::jthread(t2));
    threads.push_back(std::jthread(t3));
    threads.push_back(std::jthread(t4));

    start_point.arrive_and_wait();
    auto start = Clock::now();
    end_point.arrive_and_wait();
    auto end = Clock::now();

    for (auto &t : threads) {
      t.request_stop();
    }

    auto elapsed_seconds = std::chrono::duration_cast<fsecs>(end - start);
    state.SetIterationTime(elapsed_seconds.count());
  }

  state.counters["size"] = container_size;
  state.counters["num_threads"] = num_threads;
}

// 8 Threads
template <typename Container>
void BM_FalseSharing8SoA(benchmark::State &state) {
  Container data;
  constexpr auto num_threads = 8;

  std::barrier start_point{num_threads + 1}; // + main
  std::barrier end_point{num_threads + 1};   // + main
  auto t1 = [&]() {
    start_point.arrive_and_wait();
    for (size_t _ = 0; _ < repetitions; ++_) {
      for (size_t i = 0; i < data.vout_x1.size(); i++) {
        data.vout_x1[i] = i;
      }
    }
    end_point.arrive_and_wait();
  };

  auto t2 = [&]() {
    start_point.arrive_and_wait();
    for (size_t _ = 0; _ < repetitions; ++_) {
      for (size_t i = 0; i < data.vout_x1.size(); i++) {
        data.vout_x2[i] = i;
      }
    }
    end_point.arrive_and_wait();
  };

  auto t3 = [&]() {
    start_point.arrive_and_wait();
    for (size_t _ = 0; _ < repetitions; ++_) {
      for (size_t i = 0; i < data.vout_x1.size(); i++) {
        data.vout_x3[i] = i;
      }
    }
    end_point.arrive_and_wait();
  };

  auto t4 = [&]() {
    start_point.arrive_and_wait();
    for (size_t _ = 0; _ < repetitions; ++_) {
      for (size_t i = 0; i < data.vout_x1.size(); i++) {
        data.vout_x4[i] = i;
      }
    }
    end_point.arrive_and_wait();
  };

  auto t5 = [&]() {
    start_point.arrive_and_wait();
    for (size_t _ = 0; _ < repetitions; ++_) {
      for (size_t i = 0; i < data.vout_x1.size(); i++) {
        data.vout_x5[i] = i;
      }
    }
    end_point.arrive_and_wait();
  };

  auto t6 = [&]() {
    start_point.arrive_and_wait();
    for (size_t _ = 0; _ < repetitions; ++_) {
      for (size_t i = 0; i < data.vout_x1.size(); i++) {
        data.vout_x6[i] = i;
      }
    }
    end_point.arrive_and_wait();
  };

  auto t7 = [&]() {
    start_point.arrive_and_wait();
    for (size_t _ = 0; _ < repetitions; ++_) {
      for (size_t i = 0; i < data.vout_x1.size(); i++) {
        data.vout_x7[i] = i;
      }
    }
    end_point.arrive_and_wait();
  };

  auto t8 = [&]() {
    start_point.arrive_and_wait();
    for (size_t _ = 0; _ < repetitions; ++_) {
      for (size_t i = 0; i < data.vout_x1.size(); i++) {
        data.vout_x8[i] = i;
      }
    }
    end_point.arrive_and_wait();
  };

  for (const auto &&_ : state) {
    std::vector<std::jthread> threads;
    threads.push_back(std::jthread(t1));
    threads.push_back(std::jthread(t2));
    threads.push_back(std::jthread(t3));
    threads.push_back(std::jthread(t4));
    threads.push_back(std::jthread(t5));
    threads.push_back(std::jthread(t6));
    threads.push_back(std::jthread(t7));
    threads.push_back(std::jthread(t8));

    start_point.arrive_and_wait();
    auto start = Clock::now();
    end_point.arrive_and_wait();
    auto end = Clock::now();

    for (auto &t : threads) {
      t.request_stop();
    }

    auto elapsed_seconds = std::chrono::duration_cast<fsecs>(end - start);
    state.SetIterationTime(elapsed_seconds.count());
  }

  state.counters["size"] = container_size;
  state.counters["num_threads"] = num_threads;
}

#define BENCHMARK_ARGS Unit(benchmark::kMillisecond)->UseManualTime();

#define BENCHMARK_ALL_LAYOUTS(THREADS, TYPE)                                                                           \
  BENCHMARK_TEMPLATE(BM_FalseSharing##THREADS##AoS, AoS8Vec<TYPE, container_size>)->BENCHMARK_ARGS;                    \
  BENCHMARK_TEMPLATE(BM_FalseSharing##THREADS##SoA, SoA8Vec<TYPE, container_size>)->BENCHMARK_ARGS;

#define BENCHMARK_ALL_THREADS(TYPE)                                                                                    \
  BENCHMARK_ALL_LAYOUTS(1, TYPE)                                                                                       \
  BENCHMARK_ALL_LAYOUTS(2, TYPE)                                                                                       \
  BENCHMARK_ALL_LAYOUTS(4, TYPE)                                                                                       \
  BENCHMARK_ALL_LAYOUTS(8, TYPE)

BENCHMARK_ALL_THREADS(double);

BENCHMARK_MAIN();
