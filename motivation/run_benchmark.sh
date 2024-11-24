#!/bin/bash

compilers=("intel" "gcc" "clang")
datetime=$(date +"%Y-%m-%d_%H-%M-%S")

for cmplr in "${compilers[@]}"; do
    make clean
    rm CMakeCache.txt
    cmake . -D${cmplr}=on
    make simd
    likwid-pin -c 0 ./simd --benchmark_repetitions=10 --benchmark_report_aggregates_only=false \
    --benchmark_perf_counters=instructions --benchmark_format=json \
    | head -n -1 > "results/simd/results_${cmplr}_${1}_vec"

    make clean
    rm CMakeCache.txt
    cmake . -D${cmplr}=on -Dnovec=on
    make simd
    likwid-pin -c 0 ./simd --benchmark_repetitions=10 --benchmark_report_aggregates_only=false \
    --benchmark_perf_counters=instructions --benchmark_format=json \
    | head -n -1 > "results/simd/results_${cmplr}_${1}_novec"
    # likwid-pin -c 0 ./false_sharing --benchmark_repetitions=10 --benchmark_report_aggregates_only=false \
    # --benchmark_perf_counters= --benchmark_format=json \
    # | head -n -1 > "results/simd/results_${cmplr}_$1"

done
