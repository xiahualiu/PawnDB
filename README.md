[![Actions Status](https://github.com/xiahualiu/PawnDB/workflows/Standalone/badge.svg)](https://github.com/xiahualiu/PawnDB/actions)
[![Actions Status](https://github.com/xiahualiu/PawnDB/workflows/Test/badge.svg)](https://github.com/xiahualiu/PawnDB/actions)
[![Actions Status](https://github.com/xiahualiu/PawnDB/workflows/Style/badge.svg)](https://github.com/xiahualiu/PawnDB/actions)
[![Actions Status](https://github.com/xiahualiu/PawnDB/workflows/Aarch64/badge.svg)](https://github.com/xiahualiu/PawnDB/actions)
[![Actions Status](https://github.com/xiahualiu/PawnDB/workflows/RV64/badge.svg)](https://github.com/xiahualiu/PawnDB/actions)

# PawnDB

A lightweight, type-safe in-memory database optimized for OLTP workloads.

## Features

### Memory Management
- Zero heap allocation design
- Static memory allocation
- Built-in buffer pool management
- RAII buffer object with reference counting
- Maximum tuple size: 65535 bytes (UDP packet size limit)

### Transaction Management
- Strict 2-Phase Locking (2PL)
- Tuple-level shared/exclusive locks
- Support lock promotion
- ACID compliance
- Single record operations
- Deadlock prevention.

### Type Safety
- Compile-time type checking
- Zero dynamic dispatch
- No virtual functions
- Strong type system

## Language

* PawnDB is written in [ISO C++17](https://isocpp.org/std/the-standard), without compiler extensions.
* PawnDB follows [Google coding style](https://google.github.io/styleguide/).

## License

[MIT License](https://github.com/xiahualiu/PawnDB?tab=MIT-1-ov-file#readme)

## Build the Project

### Prerequisites

Hard requirements:

* `clang` version > 16.0, or `gcc` version > 8.0.
* `cmake` version > 3.20.
* `ninja` build system.
* `doxygen`.

You need also `clang-format` if you want to contritbute:

* `clang-format` version > 18.0. (Optional)

### Build and run the standalone target

Use the following command to build and run the executable target.

```bash
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain/clang.cmake
cmake --build build --target pawndb-app
./build/standalone/pawndb-app
```

### Build and run test suite

Use the following commands from the project's root directory to run the test suite.

```bash
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain/clang.cmake
cmake --build build --target run-all-tests
```

### Check clang-format

```bash
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain/clang.cmake
cmake --build build --target check-clang-format
```

### Apply clang-format

```bash
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain/clang.cmake
cmake --build build --target apply-clang-format
```
