[![Actions Status](https://github.com/xiahualiu/PawnDB/workflows/Library/badge.svg)](https://github.com/xiahualiu/PawnDB/actions)
[![Actions Status](https://github.com/xiahualiu/PawnDB/workflows/Standalone/badge.svg)](https://github.com/xiahualiu/PawnDB/actions)
[![Actions Status](https://github.com/xiahualiu/PawnDB/workflows/Install/badge.svg)](https://github.com/xiahualiu/PawnDB/actions)
[![Actions Status](https://github.com/xiahualiu/PawnDB/workflows/Style/badge.svg)](https://github.com/xiahualiu/PawnDB/actions)
[![Actions Status](https://github.com/xiahualiu/PawnDB/workflows/Aarch64/badge.svg)](https://github.com/xiahualiu/PawnDB/actions)
[![Actions Status](https://github.com/xiahualiu/PawnDB/workflows/RV64/badge.svg)](https://github.com/xiahualiu/PawnDB/actions)

# PawnDB (DO NOT USE, Still WIP)

PawnDB is an open source embedded DBMS that is specifically designed to meet the most rigid safety requirements from special industries such as aerospace and automotive.

It is designed based on the [Byzantine Fault Tolerant (BFT)](https://en.wikipedia.org/wiki/Byzantine_fault) algorithm to promise the maximum data reliability. You can have multiple devices running the same PawnDB software on the same IP subnet. Each PawnDB device works as a node. For any transaction the data will be committed only when more than half nodes reach a consensus at the calculated results, those nodes who fails to consent are forced to synchronize with the committed afterwards.

Despite being a DBMS, PawnDB only has minimal components so that application developers can directly add it to their application software.

## Features

- Whole database (tables & indexes) are stored **in RAM**. Most embedded systems are designed to run for extensive time (sometimes maybe >10 years), the disks (or flash chips) can easily get worn out if we contantly paging out data from RAM. In PawnDB, tables & indexes are statically allocated in RAM, meaning you need to define the tuple structure as well as the size of the table in your code. Besides, the hash index tables and hash functions also needs to be defined statically in the code.
- Fully portable, based on ISO C++ and POSIX standards.
- For concurrent transaction execution, PawnDB supports [Multi-Version Concurrency Control (MVCC)](https://en.wikipedia.org/wiki/Multiversion_concurrency_control), and [2 Phase Locking](https://en.wikipedia.org/wiki/Two-phase_locking) at the table level (to prevent phantom reads at very cheap cost).
- All tuple fields come with extra checksum values even when they are in memory, in order to prevent bit corruption.
- All nodes are identical to each other and each one of them has the capability to become the **primary proposer** at runtime, who can start a transaction to other nodes. This guarantees the whole system can run even after multiple node failure.
- PawnDB is NoSQL, since it doesn't support SQL language. However PawnDB comes with a set of common used functions (such as `SELECT` and `INSERT`) and you can also define your own function and add it to the parser. 
- Inter communication between nodes are based on the TCP/IP protocal.

## Language

* PawnDB is written in [ISO C++17](https://isocpp.org/std/the-standard).
* PawnDB follows [Google coding style](https://google.github.io/styleguide/).

## Build the Project

### Requirements

* `gcc` version > 8.0.
* `cmake` version > 3.14.
* `clang-format` version > 18.0.
* (optional) `cppcheck` version > 2.5.
* (optional) `ccache` version > 3.4.2.

`cppcheck` and `ccache` will be enabled if cmake detects them on the `PATH`.

Depending on your preference, you may install one of the following build systems:

* [Ninja](https://ninja-build.org/).
* [GNU Make](https://www.gnu.org/software/make/).

### Build and run the standalone target

Use the following command to build and run the executable target.

```bash
cmake -S . -B build
cmake --build build --target pawndb_app
./build/standalone/pawndb_app
```

### Build and run test suite

Use the following commands from the project's root directory to run the test suite.

```bash
cmake -S . -B build
cmake --build build --target pawndb_tests
cd build/test
ctest
# or simply call the executable: 
./pawndb_tests
```

### Check clang-format

```bash
cmake -S . -B build
cmake --build build --target check-clang-format
```

### Enable `ccache`

It is enable by default.