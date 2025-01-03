[![Actions Status](https://github.com/xiahualiu/PawnDB/workflows/Library/badge.svg)](https://github.com/xiahualiu/PawnDB/actions)
[![Actions Status](https://github.com/xiahualiu/PawnDB/workflows/Standalone/badge.svg)](https://github.com/xiahualiu/PawnDB/actions)
[![Actions Status](https://github.com/xiahualiu/PawnDB/workflows/Install/badge.svg)](https://github.com/xiahualiu/PawnDB/actions)
[![Actions Status](https://github.com/xiahualiu/PawnDB/workflows/Style/badge.svg)](https://github.com/xiahualiu/PawnDB/actions)
[![Actions Status](https://github.com/xiahualiu/PawnDB/workflows/Aarch64/badge.svg)](https://github.com/xiahualiu/PawnDB/actions)
[![Actions Status](https://github.com/xiahualiu/PawnDB/workflows/RV64/badge.svg)](https://github.com/xiahualiu/PawnDB/actions)

# PawnDB

PawnDB is an open source embedded DBMS that is designed to be ultra fast and secure. Some of the features of PawnDB:

#### In memory

The whole database, including a built-in buffer pool, resides in the static section of the memory. PawnDB **DOES NOT** request any heap memory from the OS, and only uses the internal buffer table to manage temporary buffers. However, this also limits the tuple size; the current PawnDB only supports tuples that are less than 65535 bytes, which is also the maximum UDP packet size on a Linux localhost network interface.

#### Lock Scheme: Strict 2-PL

PawnDB follows the strict 2-PL rule for transactions. However, there is an exception: a transaction can give up its shared lock on a tuple (only at the shrinking phase), which indicates transaction will only read a snapshot of the resources and does not care about the future values of the resources.

#### Lock Scheme: Tuple-Level Shared/Exclusive Locks

PawnDB is intended to be used as an OLTP (Online Transaction Processing) database, so it is designed with tuple-level shared/exclusive locks.

PawnDB only supports fetching/adding/removing one record at a time; this ensures it is free from the ghost record problem, which can typically be solved by adding a table lock.

#### Strong Type

PawnDB code is type-safe; all the tables and tuples must be defined in the code to use during runtime. The PawnDB code also has **NO** dynamic dispatch code, and **NO** virtual functions.

#### Portable

PawnDB code only uses C++ standard library objects, such as `std::array`, `std::mutex`, and `std::condition_variable`, etc.

There is no 3rd party library used in PawnDB, also making it suitable for commercial projects which require no restrictive copyright terms.

For the socket connection part, it uses the UNIX domain socket which is available on most POSIX systems such as various Linux distributions

#### Simple & Lightweight

PawnDB is designed to be able to run on the most tiny SoCs, including those embedded systems.

The memory footprint of PawnDB is very small, even though it has tuple level locks, the overhead of these locks are as low as ~1 bytes per tuple.

## Language

* PawnDB is written in [ISO C++17](https://isocpp.org/std/the-standard), without compiler extensions.
* PawnDB follows [Google coding style](https://google.github.io/styleguide/).

## Build the Project

### Requirements

* `clang` version > 16.0, or `gcc` version > 8.0.
* `cmake` version > 3.20.
* `clang-format` version > 18.0.
* `ninja-build`.
* (optional) `ccache` version > 3.4.2.

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
