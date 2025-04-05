# ReflMem++

Investigates usage of [C++ reflection (P2996)](https://www.wg21.link/p2996) and [token injection (P3294)](https://www.wg21.link/p3294) in modifying the memory layout of a data structure, while keeping the same view.

The goal is to have a data structure with a user-friendly interface that is independent of its memory layout. This enables **fast prototyping** of memory access patterns to optimize memory performance on different architectures (CPU, GPU).

Currently we have only implemented conversion of an Array-of-Structures (AoS) with a struct `S` to a Struct-of-Arrays (SoA), but we plan to include other layouts (e.g., AoSoA) and data reordering.

## How It Works
We provide the class `rmpp::vector<S, Alignment>`, which takes a structure `S` and specializes into an SoA version of the structure `S`. In this class, we:
   - provide a constructor `vector(size_t n)`,
   - allocate data in a single contiguous chunk of bytes, aligned to offsets of size `Alignment`,
   - define SoA members with type `std::span` that point to the byte storage,
   - and define the `operator[]`, such that the SoA can be indexed like an AoS.

We ask the user to define their structures with reference fundamental types:
```cpp
struct Vec {
   double &x, &y, &z;
};

struct S {
   int &id;
   Vec v;
};
```
Then if the user instantiates an `rmpp::vector` with the struct `S`:
```cpp
using SoA = rmpp::vector<S>;
SoA s;
```
The following parts in the class are generated at compile-time using reflection:
```cpp
namespace rmpp {
   template <typename T, size_t Alignment>
   class vector<S> {
      std::vector<std::byte storage>

      // START CODE GENERATION
      std::span<int> id;

      struct VecRef {
         std::span<double> x, y, z;
      }
      VecRef v;
      // END CODE GENERATION

      constexpr inline size_t align_size(size_t size) const {
         return ((size + Alignment - 1) / Alignment) * Alignment;
      }

      vector(size_t n) {
         // START CODE GENERATION
         m_size = n;

         // sum of the sizes of: S::id, Vec:::x, Vec::y, and Vec::z
         size_t total_byte_size = align_size(n * sizeof(int)) + align_size(n * sizeof(double))
                                  + align_size(n * sizeof(double)) + align_size(n * sizeof(double)));
         storage.resize(total_byte_size);

         id = std::span(reinterpret_cast<int *>(storage.data(), m_size))
         v.x = std::span(reinterpret_cast<double *>(storage.data() + align_size(n * sizeof(int)), m_size))
         v.y = std::span(reinterpret_cast<double *>(storage.data() + align_size(n * (sizeof(int))
                                                    + align_size(n * sizeof(double))), m_size))
         v.z = std::span(reinterpret_cast<double *>(storage.data() + align_size(n * (sizeof(int)))
                                                    + align_size(n * sizeof(double))
                                                    + align_size(n * sizeof(double)), m_size))

         // END CODE GENERATION
      }

      S operator[](std::size_t idx) const  {
         // START CODE GENERATION
         return S{ id[idx], Vec{ v.x[idx], v.y[idx], v.z[idx]} };
         // END CODE GENERATION
      }
   };
}
```
The structure can then be used as follows:
```cpp
// AoS Style Indexing
for (int i = 0; i < s.size(); i++) {
   std::cout << s[i] << std::endl;
}

// SoA Style Indexing
for (int i = 0; i < s.id.size(); i++) {
   std::cout << s.id[i] << std::endl;
}
for (int i = 0; i < s.v.x.size(); i++) {
   std::cout << s.v.x[i] << std::endl;
}
```

## Versions
So far, we implemented several versions of converting an AoS to SoA, while preserving the data access syntax of `aos[idx].x`, `aos[idx].m[i][j]`, etc.

We have the following versions in order of complexity:
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

This table summarizes what features each version supports

| **Version**            | Scalar | Vectors | (Nested Struct) | Member Methods | Contiguous Allocation |
|------------------------|--------|---------|-----------------|----------------|-----------------------|
| aos2soa                |   x    |         |                 |                |                       |
| aos2soa_contiguous     |   x    |         |                 |                |          x            |
| aos2soa_containers     |   x    |    x    |                 |                |          x            |
| with_methods           |   x    |         |        x        |       x        |          x            |


## Usage
- `edg`: contains the versions tested with the EDG experimental reflection compiler only available on [Compiler Explorer](https://godbolt.org/z/13anqE1Pa). The examples can viewed on Compiler Explorer via the following links:

| **Example**            | **GodBolt**                     |
|------------------------|---------------------------------|
| aos2soa                | https://godbolt.org/z/G4eTKW71P |
| aos2soa_contiguous     | https://godbolt.org/z/MzK5Te5z5 |
| aos2soa_containers     | https://godbolt.org/z/3bEKhr5r3 |
| with_methods           | https://godbolt.org/z/cT9soWj6s |

- `manual`: contains the manually unrolled version of each example. These can be compiled with a standard C++ compiler using CMake:
```
cmake <src-dir>
make
```