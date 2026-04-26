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

### Build in Dev Container

This repository includes a dev container in `.devcontainer/` based on Ubuntu,
with `cmake`, `ninja`, `clang/gcc`, `doxygen`, `gcovr`, `pre-commit`, `gh`,
and Node.js (for Claude Code and Continue extensions).

#### Step 1: Create Env File

Runtime also needs API keys for Claude Code and Continue. Create `.devcontainer/.env` (it is gitignored):

```bash
# Create .devcontainer/.env
ANTHROPIC_AUTH_TOKEN=<your-deepseek-api-key>
CONTINUE_API_KEY=<your-deepseek-api-key>
```

#### Step 2: Export GitHub Token

The GitHub token is needed as a BuildKit secret during image build, so export it in your shell:

```bash
export GH_TOKEN=<your-github-token>
```

#### Step 3: Build the Image

```bash
docker build \
    -f .devcontainer/Dockerfile \
    --tag pawndb-dev:latest \
    --secret id=gh_token,env=GH_TOKEN \
    .
```

#### Step 4: Open in Dev Container

Open this repository in VS Code and run:

>Dev Containers: Rebuild and Reopen in Container

Or build from CLI with BuildKit:

```bash
docker build \
	-f .devcontainer/Dockerfile \
	--tag=pawndb-dev:latest \
	--secret id=gh_token,env=GH_TOKEN \
	.
```

#### Step 5: First-Time Setup

If this is your first time, clone the repo into the workspace folder and build:

```bash
cd /workspaces
# Change volume owner to ubuntu, by default it is root
sudo chown -R ubuntu:ubuntu /workspaces/*

# Populate the volume with this git repo
git clone --recurse-submodules https://github.com/xiahualiu/PawnDB.git PawnDB

# Install pre-commit hook
cd PawnDB
pre-commit install

# Build for the first time
cmake -S . -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/clang.cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build --target pawndb-app
```

### Prerequisites

Hard requirements:

* `clang` version > 16.0, or `gcc` version > 8.0.
* `cmake` version > 3.20.
* `ninja` build system.

* `doxygen` for generating document html. (Optional)
* `gcovr` for showing test coverage report. (Optional)
* `clang-format` version > 18.0. (Optional)

### Build and run the CLI target

Use the following command to build and run the executable target.

```bash
cmake -S . -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/clang.cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build --target pawndb-app
./build/apps/pawndb-cli/pawndb-app
```

### Build and run test suite

Use the following commands from the project's root directory to run the test suite.

```bash
cmake -S . -B build -G Ninja -DPAWNDB_ENABLE_TEST=ON -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/clang.cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build --target clean-coverage
cmake --build build --target run-all-tests
cmake --build build --target show-test-coverage
```

You can run a specific test target, for example:

```bash
cmake --build build --target parser-test
./build/tests-unit/parser/parser-test
```

### Check clang-format

```bash
cmake -S . -B build -G Ninja -DPAWNDB_ENABLE_STYLE=ON -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/clang.cmake
cmake --build build --target check-clang-format
```

### Apply clang-format

```bash
cmake -S . -B build -G Ninja -DPAWNDB_ENABLE_STYLE=ON -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/clang.cmake
cmake --build build --target apply-clang-format
```

### Pre-commit hook (format check)

This repo includes a `.pre-commit-config.yaml` hook that runs the CMake
`check-clang-format` target before each commit.

In the dev container, `pre-commit` is preinstalled via apt:

Run it manually on all files:

```bash
pre-commit run --all-files
```

### Build Docs

```bash
cmake -S . -B build -G Ninja -DPAWNDB_ENABLE_DOXYGEN=ON -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/clang.cmake
cmake --build build --target doxygen
```
