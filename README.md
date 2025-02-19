# ReflMem++

Investigates usage of [C++ reflection (P2996)](https://www.wg21.link/p2996) and [token injection (P3294)](https://www.wg21.link/p3294) in modifying the memory layout of a data structure, while keeping the same view.

The goal is to have a data structure with a user-friendly interface that is independent of its memory layout. This enables fast prototyping of memory access patterns and optimal memory performance on different architectures.

## Examples
So far, we implemented several examples of converting an Array-of-Structures (AoS) with a struct `S` to a Struct-of-Arrays (SoA) type, while preserving the data access syntax of `aos[idx].x`, `aos[idx].m[i][j]`, etc.
### aos2soa
1) Struct `S` contains four `double` data members.
2) The converted SoA stores the values for each member in a separate `std::vector`.
### aos2soa_contiguous
1) Struct `S` contains four `double` data members.
2) The SoA storage is allocated in a single contiguous chunk
3) The SoA data members are of type `std::span`, which are contiguously mapped to the storage buffer.
### aos2soa_containers
1) Struct `S` contains `double`, `std::vector`, and `std::array<std::array>` (matrix) data members.
2) The SoA storage is allocated in a single contiguous chunk
3) The SoA data members are of type `std::span`, which are contiguously mapped to the storage buffer.
4) The data for the vector member is assigned contiguously like `aos[0].v[0], ..., aos[0].v[v_size-1], aos[1].v[0], ..., aos[1].v[v_size-1], etc.`
5) The data for the matrix member is assigned contiguously element-wise like `aos[0].m[0][0], ..., aos[n_elem].m[0][0], aos[0].m[0][1], ..., aos[n_elem].m[0][1], etc.` AoS access to the matrix data member is implemented using `mdspan`.
### with_methods
1) Struct `S` contains a scalar `double` member and nested struct members.
2) The SoA storage is allocated in a single contiguous chunk.
3) The SoA data members are of type `std::span`, which are contiguously mapped to the storage buffer.
4) Nested struct members are recursively transformed into SoAs with `std::span` members that point to the storage
   buffer in the parent struct.

## Usage
- `edg`: contains the examples tested with the EDG experimental reflection compiler available on [Compiler Explorer](https://godbolt.org/z/13anqE1Pa). The examples can viewed on Compiler Explorer via the following links:

| **Example**            | **GodBolt**                     |
|------------------------|---------------------------------|
| aos2soa                | https://godbolt.org/z/7fWcoazTs |
| aos2soa_contiguous     | https://godbolt.org/z/6xb3PE4e5 |
| aos2soa_containers     | https://godbolt.org/z/9s18G6WP1 |
| with_methods           | https://godbolt.org/z/4EfqE81Y4 |

- `manual`: contains the manually unrolled versions of each example. These can be compiled with a standard C++ compiler using CMake:
```
cmake <src-dir>
make
```