#!/bin/bash

compilers=("intel" "gcc" "clang")
datetime=$(date +"%Y-%m-%d_%H-%M-%S")

for cmplr in "${compilers[@]}"; do
    make clean
    rm CMakeCache.txt
    cmake . -D${cmplr}=on
    make aos2soa
    likwid-pin -q -c 0 ./aos2soa --benchmark_repetitions=10 --benchmark_report_aggregates_only=false \
    --benchmark_perf_counters=instructions --benchmark_format=json > "results/aos2soa/results_${cmplr}_${1}_vec"

    make clean
    rm CMakeCache.txt
    cmake . -D${cmplr}=on -Dnovec=on
    make aos2soa
    likwid-pin -q -c 0 ./aos2soa --benchmark_repetitions=10 --benchmark_report_aggregates_only=false \
    --benchmark_perf_counters=instructions --benchmark_format=json > "results/aos2soa/results_${cmplr}_${1}_novec"
done

make clean
rm CMakeCache.txt
cmake . -Dgcc=on
make false_sharing
likwid-pin -q -c 0-16 ./false_sharing --benchmark_repetitions=10 --benchmark_report_aggregates_only=false \
    --benchmark_format=json > "results/false_sharing/results_$1"
