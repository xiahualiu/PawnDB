[![Actions Status](https://github.com/xiahualiu/PawnDB/workflows/MacOS/badge.svg)](https://github.com/xiahualiu/PawnDB/actions)
[![Actions Status](https://github.com/xiahualiu/PawnDB/workflows/Windows/badge.svg)](https://github.com/xiahualiu/PawnDB/actions)
[![Actions Status](https://github.com/xiahualiu/PawnDB/workflows/Ubuntu/badge.svg)](https://github.com/xiahualiu/PawnDB/actions)
[![Actions Status](https://github.com/xiahualiu/PawnDB/workflows/Style/badge.svg)](https://github.com/xiahualiu/PawnDB/actions)
[![Actions Status](https://github.com/xiahualiu/PawnDB/workflows/Install/badge.svg)](https://github.com/xiahualiu/PawnDB/actions)

# PawnDB (DO NOT USE, Still WIP)

PawnDB is an open source embedded DBMS that is specifically designed to meet the most rigid safety requirements from special industries such as aerospace and automotive.

It is designed based on the [Byzantine Fault Tolerant (BFT)](https://en.wikipedia.org/wiki/Byzantine_fault) algorithm to promise the maximum data reliability. You can have multiple devices running the same PawnDB software on the same IP subnet. Each PawnDB device works as a node. For any transaction the data will be committed only when more than half nodes reach a consensus at the calculated results, those nodes who fails to consent are forced to be synchronized afterwards.

Compared to other modern database management systems, the functions PawnDB provides are pretty limited. Despite being a DBMS, PawnDB only has minimal components so that application developers can directly add it to their application software.

## Features

- Whole database (tables & indexes) are stored **in RAM**. Most embedded systems are designed to run for extensive time (sometimes maybe >10 years), the disks (or flash chips) can easily get worn out if we contantly paging out data from RAM. In PawnDB, tables & indexes are statically allocated in RAM, meaning you need to define the tuple structure as well as the size of the table in your code. Besides, the hash index tables and hash functions also needs to be defined statically in the code.
- For concurrent transaction execution, PawnDB supports [Multi-Version Concurrency Control (MVCC)](https://en.wikipedia.org/wiki/Multiversion_concurrency_control), and [2 Phase Locking](https://en.wikipedia.org/wiki/Two-phase_locking) at the table level (to prevent phantom reads at very cheap cost).
- All tuple fields come with extra checksum values even when they are in memory, in order to prevent bit corruption.
- All nodes are identical to each other and each one of them has the capability to become the **primary proposer** at runtime, who can start a transaction to other nodes. This guarantees the whole system can run even after multiple node failure.
- PawnDB is NoSQL, since it doesn't support SQL language. However PawnDB comes with a set of common used functions (such as `SELECT` and `INSERT`) and you can also define your own function and add it to the parser. 
- Inter communication between nodes are based on the TCP/IP protocal.

## Language

* PawnDB is written in [ISO C++17](https://isocpp.org/std/the-standard).
* PawnDB follows [Google coding style](https://google.github.io/styleguide/).

## Build the Project

### Build and run the standalone target

Use the following command to build and run the executable target.

```bash
cmake -S standalone -B build/standalone
cmake --build build/standalone
./build/standalone/Greeter --help
```

### Build and run test suite

Use the following commands from the project's root directory to run the test suite.

```bash
cmake -S test -B build/test
cmake --build build/test
CTEST_OUTPUT_ON_FAILURE=1 cmake --build build/test --target test

# or simply call the executable: 
./build/test/GreeterTests
```

To collect code coverage information, run CMake with the `-DENABLE_TEST_COVERAGE=1` option.

### Run clang-format

Use the following commands from the project's root directory to check and fix C++ and CMake source style.
This requires _clang-format_, _cmake-format_ and _pyyaml_ to be installed on the current system.

```bash
cmake -S test -B build/test

# view changes
cmake --build build/test --target format

# apply changes
cmake --build build/test --target fix-format
```

See [Format.cmake](https://github.com/TheLartians/Format.cmake) for details.
These dependencies can be easily installed using pip.

```bash
pip install clang-format==14.0.6 cmake_format==0.6.11 pyyaml
```

### Build the documentation

The documentation is automatically built and [published](https://thelartians.github.io/ModernCppStarter) whenever a [GitHub Release](https://help.github.com/en/github/administering-a-repository/managing-releases-in-a-repository) is created.
To manually build documentation, call the following command.

```bash
cmake -S documentation -B build/doc
cmake --build build/doc --target GenerateDocs
# view the docs
open build/doc/doxygen/html/index.html
```

To build the documentation locally, you will need Doxygen, jinja2 and Pygments installed on your system.

### Build everything at once

The project also includes an `all` directory that allows building all targets at the same time.
This is useful during development, as it exposes all subprojects to your IDE and avoids redundant builds of the library.

```bash
cmake -S all -B build
cmake --build build

# run tests
./build/test/GreeterTests
# format code
cmake --build build --target fix-format
# run standalone
./build/standalone/Greeter --help
# build docs
cmake --build build --target GenerateDocs
```

### Additional tools

The test and standalone subprojects include the [tools.cmake](cmake/tools.cmake) file which is used to import additional tools on-demand through CMake configuration arguments.
The following are currently supported.

#### Sanitizers

Sanitizers can be enabled by configuring CMake with `-DUSE_SANITIZER=<Address | Memory | MemoryWithOrigins | Undefined | Thread | Leak | 'Address;Undefined'>`.

#### Static Analyzers

Static Analyzers can be enabled by setting `-DUSE_STATIC_ANALYZER=<clang-tidy | iwyu | cppcheck>`, or a combination of those in quotation marks, separated by semicolons.
By default, analyzers will automatically find configuration files such as `.clang-format`.
Additional arguments can be passed to the analyzers by setting the `CLANG_TIDY_ARGS`, `IWYU_ARGS` or `CPPCHECK_ARGS` variables.

#### Ccache

Ccache can be enabled by configuring with `-DUSE_CCACHE=<ON | OFF>`.