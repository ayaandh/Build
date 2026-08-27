# Build

A fast, minimal C++ build tool designed to make compiling C++ projects simple.

Build wraps the boring parts of invoking a compiler into a small, straightforward command-line tool. No massive project files, no complicated configuration system, and no need to remember a ridiculous collection of compiler commands.

## Features

* Compile multiple C++ source files
* Automatic wildcard expansion
* Recursive source discovery
* Custom compiler flags
* Custom output paths
* Run programs immediately after building
* Clean build outputs
* Dry-run mode
* Verbose output
* Runtime arguments
* Automatic `.exe` handling on Windows
* Automatic output-directory creation
* C++ source detection for `.cpp`, `.cc`, `.cxx`, and `.c++`
* Duplicate source removal
* Simple command-line interface

## Installation

Clone the repository:

```bash
git clone [<repository-url>](https://github.com/ayaandh/Build.git)
cd build
```

Compile Build:

```bash
g++ src/main.cpp -std=c++20 -O2 -o build.exe
```

Move `build.exe` somewhere on your `PATH`.

Verify the installation:

```bash
build --version
```

## Usage

The basic syntax is:

```text
build <sources...> <output> [options]
```

You can also explicitly specify the output:

```text
build <sources...> -o <output> [options]
```

### Single source file

```bash
build main.cpp app
```

This compiles:

```text
main.cpp
```

into:

```text
app.exe
```

### Multiple source files

```bash
build main.cpp player.cpp world.cpp game
```

### Wildcards

Build supports wildcard source patterns:

```bash
build *.cpp app
```

You can also specify a directory:

```bash
build src/*.cpp app
```

### Recursive source discovery

Use `-r` or `--recursive`:

```bash
build src/*.cpp app --recursive
```

This searches subdirectories as well.

## Compiler Options

Compiler options can be passed directly to Build.

```bash
build *.cpp app -Wall -Wextra -Wpedantic
```

For optimization:

```bash
build *.cpp app -O2
```

For debugging:

```bash
build *.cpp app -g
```

To select a C++ standard:

```bash
build *.cpp app -std=c++23
```

Include directories and libraries can also be passed directly:

```bash
build *.cpp app -Iinclude -Llib -lmylib
```

Build passes these options directly to `g++`.

## Running Programs

Use `-run` or `--run` to run the executable after a successful build:

```bash
build main.cpp app --run
```

Runtime arguments can be passed after `--`:

```bash
build main.cpp app --run -- hello 123
```

Everything after `--` is treated as an argument for the compiled program.

For example:

```bash
build main.cpp app -- hello 123
```

results in the equivalent of:

```text
app.exe hello 123
```

## Output

The output executable can be specified with `-o`:

```bash
build main.cpp -o bin/app
```

Build automatically adds `.exe` when necessary on Windows:

```text
bin/app
```

becomes:

```text
bin/app.exe
```

Output directories are created automatically if they don't exist.

For example:

```bash
build src/*.cpp -o bin/debug/game
```

creates the required directories before compiling.

## Cleaning

Remove the generated executable:

```bash
build main.cpp app --clean
```

This removes:

```text
app.exe
```

without compiling anything.

## Dry Run

Use `--dry-run` to see what Build would execute without actually compiling:

```bash
build *.cpp app --dry-run
```

This is useful for inspecting the generated compiler command.

## Verbose Mode

Use `-v` or `--verbose` to display additional information:

```bash
build *.cpp app --verbose
```

Build will display the discovered source files and output path before compilation.

## Help

Display the complete command reference:

```bash
build --help
```

## Version

Display the installed Build version:

```bash
build --version
```

## Examples

### Basic project

```text
project/
├── main.cpp
├── player.cpp
└── world.cpp
```

Build it with:

```bash
build *.cpp game
```

### Project with headers

```text
project/
├── include/
│   ├── player.hpp
│   └── world.hpp
├── src/
│   ├── main.cpp
│   ├── player.cpp
│   └── world.cpp
```

Build it with:

```bash
build src/*.cpp game -Iinclude
```

### Debug build

```bash
build src/*.cpp game -Iinclude -g -Wall -Wextra
```

### Optimized build

```bash
build src/*.cpp game -Iinclude -O2 -DNDEBUG
```

### Build and run

```bash
build src/*.cpp game -Iinclude --run
```

### Build and pass arguments

```bash
build src/*.cpp game --run -- level 3
```

## Command Reference

| Option                  | Description                             |
| ----------------------- | --------------------------------------- |
| `-o`, `--output <file>` | Specify the output executable           |
| `-run`, `--run`         | Run the executable after building       |
| `--clean`               | Remove the output executable            |
| `-r`, `--recursive`     | Search wildcard patterns recursively    |
| `-v`, `--verbose`       | Display detailed build information      |
| `--dry-run`             | Display commands without executing them |
| `-h`, `--help`          | Display help                            |
| `--version`             | Display the Build version               |
| `--`                    | Pass remaining arguments to the program |

Any unrecognized option beginning with `-` is passed directly to `g++`.

## Requirements

* C++20 or newer
* `g++`
* Windows or another platform supported by the C++ standard library implementation

Build uses:

```cpp
#include <filesystem>
```

so a compiler with C++17 filesystem support is required at minimum, although Build itself is intended to be compiled with C++20 or newer.

## Design

Build intentionally keeps its interface small.

Instead of requiring a large configuration file for a simple project:

```text
build src/*.cpp mygame -std=c++23 -O2 -Wall
```

is enough.

The goal is to make common C++ builds require almost no configuration while still allowing compiler arguments to be passed through when more control is needed.

## Project Status

Build is currently a lightweight command-line build tool.

The project is designed to grow toward a more complete C++ build system while keeping its command-line interface simple and predictable.

## License

Build is licensed under the [MIT License](LICENSE).

Built by [Ayaan Dhalait](https://github.com/ayaandh)

See the repository license for details.
