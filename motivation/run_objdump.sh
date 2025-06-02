#!/bin/bash

rm CMakeCache.txt && cmake . $1
make clean && cmake --build . --verbose
objdump -d -Mx86-64 --no-addresses --no-show-raw-insn aos2soa > "asm/random/aos2soa-${2}.s"
# llvm-mca-18 --all-stats  -march=x86-64 -o "asm/aos2soa-${2}.mca" "asm/aos2soa-${2}.s"
