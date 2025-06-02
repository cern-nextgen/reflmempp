#!/bin/bash

for file in asm/random/*focus.s;
do
  # Extract the base name of the file (without the directory and extension)
  base_name=$(basename "$file" .s)

  # Run llvm-mca-18 on the file and output the results to a .mca file
  llvm-mca-18 --all-stats -march=x86-64 -o "asm/random/$base_name.mca" "$file"
done