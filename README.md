# Custom Memory Pool Allocator & Smart Pointers

A from-scratch implementation of a fixed-size block pool allocator and smart pointers (`unique_ptr`, `shared_ptr`) in modern C++, designed for performance-critical allocation patterns.

## Features

- **Pool Allocator** — Pre-allocates a contiguous memory block and serves fixed-size chunks via a free-list, achieving O(1) allocation and deallocation
- **unique_ptr** — Exclusive-ownership smart pointer with move semantics, custom deleter support, and disabled copy semantics
- **shared_ptr** — Shared-ownership smart pointer with reference-counted control blocks, type-erased custom deleters via virtual dispatch, and full copy/move support
- **Pool Integration** — `make_unique_from_pool` and `make_shared_from_pool` factory functions that allocate from the pool using placement new and return memory via RAII-based lifecycle management

## Project Structure

```
memory_pool/
├── CMakeLists.txt
├── include/
│   └── memory_pool/
│       ├── PoolAllocator.h
│       ├── UniquePtr.h
│       ├── SharedPtr.h
│       └── PoolDeleter.h
└── src/
    └── PoolAllocator.cpp
```

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Usage

### Pool Allocator with unique_ptr

```cpp
PoolAllocator pool(sizeof(MyObject), 100);

auto ptr = make_unique_from_pool<MyObject>(pool, arg1, arg2);
ptr->doSomething();
// memory returned to pool when ptr goes out of scope
```

### Pool Allocator with shared_ptr

```cpp
PoolAllocator pool(sizeof(MyObject), 100);

auto sp1 = make_shared_from_pool<MyObject>(pool, arg1, arg2);
auto sp2 = sp1; // ref count = 2
sp1.reset();    // ref count = 1, object still alive
// object destroyed and memory returned to pool when sp2 goes out of scope
```

## How It Works

**PoolAllocator** allocates a single `char[]` buffer upfront and partitions it into fixed-size blocks. Each free block is treated as a node in an intrusive singly-linked free list using `reinterpret_cast`. `allocate()` pops from the head and `deallocate()` pushes back — both O(1).

**unique_ptr** enforces exclusive ownership by deleting copy operations and implementing move semantics. Accepts a custom `Deleter` template parameter.

**shared_ptr** uses a polymorphic `ControlBlock` hierarchy for type-erased deletion:
- `DefaultControlBlock<T>` — calls `delete`
- `DeleterControlBlock<T, Deleter>` — calls a custom deleter (e.g. `PoolDeleter`)

Reference counting is managed via `shared_count` in the control block. The object is destroyed when the count reaches zero.

**PoolDeleter** explicitly calls the object's destructor (`ptr->~T()`) and returns the raw memory to the pool via `deallocate()`, enabling full RAII integration between smart pointers and the pool allocator.
