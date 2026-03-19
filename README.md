# ReflMem++

A research project that investigates the usage of [C++ reflection (P2996)](https://www.wg21.link/p2996) and [token injection (P3294)](https://www.wg21.link/p3294) in modifying the memory layout of a data structure, while keeping the same view.

The goal is to have a data structure with a user-friendly interface that is independent of its memory layout. This enables **fast prototyping** of memory access patterns to optimize memory performance on different architectures.

Currently we have only implemented conversion of an Array-of-Structures (AoS) with a struct `S` to a Struct-of-Arrays (SoA), but we plan to include other layouts (e.g., AoSoA) and data reordering.

# Project Structure
- `clang-p2996`: contains different methods tested with Clang's P2996 compiler.
- `edg`: contains a conversion method using token sequence injection and can only be compiled with the EDG reflection compiler.
- `manual`: contains a manually written examples of what code is generated for each method in `clang-p2996` and `edg`.

## Publications & Presentations
- *C++ Reflection for Data Layout Abstraction*. February 18, 2026. Kokkos tea-time. https://cexa-project.org/kokkos-tea-time/2026-02-18-tea-time-c++-reflection/
- Jolly Chen, Ana Lucia Varbanescu, and Axel Naumann. 2025. *Optimizing Memory Access Patterns through Automatic Data Layout Transformation (Work in Progress Paper)*. In Companion of the 16th ACM/SPEC International Conference on Performance Engineering (ICPE '25). Association for Computing Machinery, New York, NY, USA, 47–53. https://doi.org/10.1145/3680256.3722203