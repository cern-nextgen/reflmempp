# ReflMem++

Investigates usage of [C++ reflection (P2996)](https://www.wg21.link/p2996) and [token injection (P3294)](https://www.wg21.link/p3294) in modifying the memory layout of a data structure, while keeping the same view.

The goal is to have a data structure with a user-friendly interface that is independent of its memory layout. This enables fast prototyping of memory access patterns and optimal memory performance on different architectures.

## Examples
So far, we implemented several examples of converting an Array-of-Structures (AoS) to a Struct-of-Arrays (SoA), while preserving the data access syntax of `aos[idx].x`, `aos[idx].m[i][j]`, etc.
### aos2soa
Struct contains for `double` data members. The converted SoA stores the values for each member in a separate `std::vector`.
### aos2soa_contiguous
Struct contains for `double` data members. The SoA storage is allocated in a single contiguous chunk, and the data vectors for the members are contiguously mapped to this storage buffer using `std::span`.
### aos2soa_containers
Struct contains `double`, `std::vector`, and `std::array<std::array>` (matrix) data members.
The SoA storage is again allocated in a single contiguous chunk with member spans mapped to it.

The data for the vector member is assigned contiguously like `aos[0].v[0], ..., aos[0].v[v_size-1], aos[1].v[0], ..., aos[1].v[v_size-1], etc.`

The data for the matrix member is assigned contiguously element-wise like `aos[0].m[0][0], ..., aos[n_elem].m[0][0], aos[0].m[0][1], ..., aos[n_elem].m[0][1], etc.` AoS access to the matrix data member is implemented using `mdspan`.

## Usage
- `edg`: contains the examples tested with the EDG experimental reflection compiler available on [Compiler Explorer](https://godbolt.org/z/13anqE1Pa). The examples can viewed on Compiler Explorer via the following links:

| **Example**            | **GodBolt**                     |
|------------------------|---------------------------------|
| aos2soa                | https://godbolt.org/z/7fWcoazTs |
| aos2soa_contiguous     | https://godbolt.org/z/6xb3PE4e5 |
| aos2soa_containers     | https://godbolt.org/z/9s18G6WP1 |

- `manual`: contains the manually unrolled versions of each example. These can be compiled with a standard C++ compiler using CMake:
```
cmake <src-dir>
make
```