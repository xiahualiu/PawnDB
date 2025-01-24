[![Actions Status](https://github.com/xiahualiu/PawnDB/workflows/Standalone/badge.svg)](https://github.com/xiahualiu/PawnDB/actions)
[![Actions Status](https://github.com/xiahualiu/PawnDB/workflows/Test/badge.svg)](https://github.com/xiahualiu/PawnDB/actions)
[![Actions Status](https://github.com/xiahualiu/PawnDB/workflows/Style/badge.svg)](https://github.com/xiahualiu/PawnDB/actions)
[![Actions Status](https://github.com/xiahualiu/PawnDB/workflows/Aarch64/badge.svg)](https://github.com/xiahualiu/PawnDB/actions)
[![Actions Status](https://github.com/xiahualiu/PawnDB/workflows/RV64/badge.svg)](https://github.com/xiahualiu/PawnDB/actions)

# PawnDB

An **ultra lightweight & fast**, **portable** & **type-safe** in-memory database optimized for OLTP workloads.

[PawnDB Wiki](https://github.com/xiahualiu/PawnDB/wiki)

[PawnDB Doxygen Docs](https://xiahualiu.github.io/PawnDB/)

## Language

* PawnDB is written in [ISO C++17](https://isocpp.org/std/the-standard), without any compiler extensions.
* PawnDB follows [Google coding style](https://google.github.io/styleguide/).

## License

[MIT License](https://github.com/xiahualiu/PawnDB?tab=MIT-1-ov-file#readme)

## Build the Project

### Prerequisites

Hard requirements:

* `clang` version > 16.0, or `gcc` version > 8.0.
* `cmake` version > 3.20.
* `ninja` build system.

* `doxygen` for generating document html. (Optional)
* `gcovr` for showing test coverage report. (Optional)
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
cmake -S . -B build -DPAWNDB_ENABLE_TEST -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain/clang.cmake
cmake --build build --target clean-coverage
cmake --build build --target run-all-tests
cmake --build build --target show-test-coverage

```

### Check clang-format

```bash
cmake -S . -B build -DPAWNDB_ENABLE_STYLE -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain/clang.cmake
cmake --build build --target check-clang-format
```

### Apply clang-format

```bash
cmake -S . -B build -DPAWNDB_ENABLE_STYLE -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain/clang.cmake
cmake --build build --target apply-clang-format
```

### Build Docs

```bash
cmake -S . -B build -DPAWNDB_ENABLE_DOXYGEN -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain/clang.cmake
cmake --build build --target doxygen
```
