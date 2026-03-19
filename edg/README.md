
## How It Works
We provide the class `rmpp::AoS2SoA<S, Alignment>`, which takes a structure `S` and specializes into an SoA version of the structure `S`. In this class, we:
   - provide a constructor `AoS2SoA(size_t n)`,
   - allocate data in a single contiguous chunk of bytes, aligned to offsets of size `Alignment`,
   - define SoA members with type `std::span` that point to the byte storage,
     - Nested struct members are recursively transformed into SoAs
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
Then if the user instantiates an `rmpp::AoS2SoA` with the struct `S`:
```cpp
using SoA = rmpp::AoS2SoA<S>;
SoA s;
```
This is what will be generated using reflection (see also `manual/aos2soa.cpp`):
```cpp
namespace rmpp {
   template <typename T, size_t Alignment>
   class AoS2SoA<S> {
      std::span<std::byte storage>

      // START GENERATED CODE
      std::span<int> id;

      struct VecSoA {
         std::span<double> x, y, z;
      }
      VecSoA v;
      // END GENERATED CODE

      constexpr inline size_t align_size(size_t size) const {
         return ((size + Alignment - 1) / Alignment) * Alignment;
      }

      AoS2SoA(std::byte *buf, const size_t buf_size, const size_t capacity) {
         storage = std::span<std::byte>(buf, buf_size);
         m_size = 0;
         m_capacity = capacity;

         // START GENERATED CODE
         m_size = n;

         // sum of the sizes of: S::id, Vec:::x, Vec::y, and Vec::z
         id = std::span(reinterpret_cast<int *>(storage.data(), m_size))
         v.x = std::span(reinterpret_cast<double *>(storage.data() + align_size(id.size_bytes()), m_size))
         v.y = std::span(reinterpret_cast<double *>(storage.data() + align_size(id.size_bytes())
                                                    + align_size(v.x.size_bytes()), m_size))
         v.z = std::span(reinterpret_cast<double *>(storage.data() + align_size(id.size_bytes())
                                                                   + align_size(v.x.size_bytes())
                                                                   + align_size(v.y.size_bytes()), m_size))

         // END GENERATED CODE
      }

      S operator[](std::size_t idx) const  {
         // START GENERATED CODE
         return S{ id[idx], Vec{ v.x[idx], v.y[idx], v.z[idx]} };
         // END GENERATED CODE
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

## Usage
- `aos2soa.cpp`: contains the versions tested with the EDG experimental reflection compiler only available on [Compiler Explorer](https://godbolt.org/z/13anqE1Pa). The implementation can viewed on Compiler Explorer via the following link: https://godbolt.org/z/YY1qdfv3o

- `../manual/edg_aos2soa_norefl`: contains the manually unrolled version of each example. These can be compiled with a standard C++ compiler using CMake.