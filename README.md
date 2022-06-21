# Simplest possible C++ Hello World with CMake and Conan

Often in projects its nice to have high-level build scripts that hide
underlying build system.  Heck, some people just like to click on a
little green arrow or little green bug 🪲and hope for the best!

But it's also nice to understand how one would proceed from scratch
without relying on them.  This levels the playing field when the
little green button malfunctions or one needs to switch tools.  

It pays to understand what goes one "under the hood". To a certain level of course.  

Here, that level are the configuration files for CMake, a [build
automation tool](https://cmake.org) and Conan, a [C++ package
manager](https://conan.io/).  Will be assuming those are installed and
that usual things like a C++ compiler (`clang++`), a shell (`bash` or
`zsh`) and `git` are installed.

## Make four source files

So we have four source files: `src/hello.cpp`, `CMakeLists.txt`,
`conanfile.txt` and `.gitignore`.

```c++
// src/hello.cpp
#include <iostream>
#include <vector>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

int main(int argc, char* argv[]) {
  std::vector args(argv, argv + argc);
  json j{{"Hello", "World"}, {"args", args}};
  std::cout << j << "\n";
}
```

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.9)
project(hello CXX) # PROJECT_NAME is now "hello"
set(CMAKE_CXX_STANDARD 20)

# This will come in handy for LSP servers such as clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Decent compile options
add_compile_options(-Wall -Wextra -Werror -pedantic)

# Conan is important
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

# Make the "hello" executable target. Use globbing for src
file(GLOB_RECURSE src_cpp CONFIGURE_DEPENDS "src/*.cpp")
add_executable(${PROJECT_NAME} ${src_cpp})

# Add -DHELLO_IS_HELLO preprocessor define
target_compile_definitions(${PROJECT_NAME} PUBLIC HELLO_IS_HELLO)
```

```conf
# conanfile.txt
[requires]
nlohmann_json/3.10.5

[generators]
cmake
```

```.gitignore
# The all-important .gitignore
build*/
.cache
compile_commands.json
```

## Make a build subdirectory

```sh
mkdir build
cd build
```

This will house lots of generated artifacts that _shouldn't_ be
checked in.  There can be many `build-*` directories, one for each
different type of [build
configuration](https://cmake.org/cmake/help/latest/manual/cmake-buildsystem.7.html#build-configurations).

Our `.gitignore` makes sure Git doesn't see them, so files in them
will never be checked in.  It's to wipe them out and re-make them at
any time.

## Tell Conan to install packages, maybe build them

Do this inside the newly made `build` directory.

```sh
# inside hello-world/build
conan install --build=missing ../
```

The `--build=missing` tells conan to automatically download and build
any needed dependencies.  Any built packages are _not_ stored in the
currect directory (iow, there is no `node modules`-like).  Instead
Conan uses
its own cache and doesn't redo any builds it doesn't need to.  

Beware that Conan uses compiler-specific _profiles_.  The `default`
profile is defined in `~/.conan/profiles/default` and it will say
something like this:

```conf
[settings]
os=Linux
os_build=Linux
arch=x86_64
arch_build=x86_64
compiler=clang
compiler.version=13
compiler.libcxx=libstdc++11
build_type=Release
[options]
[build_requires]
[env]
```

As I'm writing this, it seems that the compilers used by Conan and
CMake **have to match** so one can link one's program with the
Conan-built libs.  In my example, I setup Conan to use the `clang`
family of compilers (actually `/usr/bin/clang++`).  Also see [this
Conan question](https://github.com/conan-io/conan/issues/1211) for
more options.

## In the build subdir, tell CMake to generate build files

By default, it will use GNU Make Makefile, which is fine for small
projects.

```sh
# inside hello-world/build
CXX=clang++ cmake ../
```

Notice the very important `CXX=clang++`.  It will only take effect if
CMake has _never run in the directory_. The following lines should be
echoed back by CMake.

```
-- The CXX compiler identification is Clang 13.0.1
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /usr/bin/clang++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Conan: Adjusting output directories
-- Conan: Using cmake global configuration
-- Conan: Adjusting default RPATHs Conan policies
-- Conan: Adjusting language standard
-- Current conanbuildinfo.cmake directory: $HOME/Source/Cpp/hello-world-2000/build
-- Conan: Compiler Clang>=8, checking major version 13
-- Conan: Checking correct version: 13
-- Configuring done
-- Generating done
-- Build files have been written to: $HOME/Source/Cpp/hello-world-2000/build
```

## Almost Done! Now just type `make`

Makefiles were generated, because the default CMake generator is
`-G"Unix Makefiles"`.  It could be `-GNinja` if one has
[Ninja](https://ninja-build.org/) installed.

```sh
# inside hello-world/build
make
```

## Run this masterpiece of program

```sh
# inside hello-world/build
bin/hello madarfacar
{"Hello":"World","args":["bin/hello","madarfacar"]}
```

## Enable use of a LSP language server like `clangd`

Clangd is a LSP server, a great tool for in-editor language support.
I use it a lot with the [Eglot LSP
client](https://github.com/joaotavora/eglot).  It works best when it
knows about the compilation flags, and the usual way to feed it that
is via a `compile_commands.json` file, which CMake happens to have
generated for us inside `hello-world/build`.  So it suffices to link
it to the root of the project.  Here's one way to do it

```sh
# inside hello-world/build
cd ..
# now inside hello-world/
ln -sf build/compile_commands.json
```

If one is in Emacs and has Eglot installed, typing `M-x eglot` in the
`hello.cpp` file should automatically pick up `clangd` and give rich
navigation and documentation facilities.

## How did `make` compile my program?

`make VERBOSE=1` shows how `make` is invoking the compiler and linker.
Here, this shows a lengthy command telling `clang++` where to find
Conan's secretly built package.  It also shows how CMake setting up
Make with a bunch of Make-specific dependency tricks (those are the
`-MD`, `-MT`, `-MF` flags, outside the score of this article).

Most importantly, it also shows us that no optimization flags were
passed to `clang++`.  Neither did it add any special debugging info.

```sh
/usr/bin/clang++ -DHELLO_IS_HELLO                           \
-I$HOME/.conan/data/nlohmann_json/3.10.5/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9/include  \
-Wall -Wextra -Werror -pedantic -std=gnu++20                \
-MD -MT CMakeFiles/hello.dir/src/hello.cpp.o                \
-MF CMakeFiles/hello.dir/src/hello.cpp.o.d                  \
-o CMakeFiles/hello.dir/src/hello.cpp.o                     \
-c $HOME/Source/Cpp/hello-world-2000/src/hello.cpp
```

## Make a Release build with -O3 and the NDEBUG preprocessor define

Go back to the project root and make a `build-release` directory.

```sh
mkdir build-release
cd build-release
conan install --build=missing ../
CXX=clang++ cmake -DCMAKE_BUILD_TYPE=Release ../
```

Typing `make VERBOSE=1` now confirms that `-O3` and `-DNDEBUG` was
used.

```
/usr/bin/clang++ -DHELLO_IS_HELLO                           \
-I$HOME/.conan/data/nlohmann_json/3.10.5/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9/include  \
-O3 -DNDEBUG                                                \
-Wall -Wextra -Werror -pedantic -std=gnu++20                \
-MD -MT CMakeFiles/hello.dir/src/hello.cpp.o                \
-MF CMakeFiles/hello.dir/src/hello.cpp.o.d                  \
-o CMakeFiles/hello.dir/src/hello.cpp.o                     \
-c $HOME/Source/Cpp/hello-world-2000/src/hello.cpp
```

## Make a Debug build with -O0 and the NDEBUG preprocessor define

Here, I recommend going back to `hello-world/build` and making that
the normal build directory for the "debug" builds.

```sh
cd ..
cd build
CXX=clang++ cmake -DCMAKE_BUILD_TYPE=Debug ../
```

Again we can confirm via `make VERBOSE=1` that the debug flag `-g` is
included.

```
/usr/bin/clang++ -DHELLO_IS_HELLO                           \
-I$HOME/.conan/data/nlohmann_json/3.10.5/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9/include  \
-g
-Wall -Wextra -Werror -pedantic -std=gnu++20                \
-MD -MT CMakeFiles/hello.dir/src/hello.cpp.o                \
-MF CMakeFiles/hello.dir/src/hello.cpp.o.d                  \
-o CMakeFiles/hello.dir/src/hello.cpp.o                     \
-c $HOME/Source/Cpp/hello-world-2000/src/hello.cpp
```

There are more build types, such as `RelWithDebInfo` and `MinSizeRel`.
It's not immediately clear how one configures flags for these build
types though I guess [this
documentation](https://cmake.org/cmake/help/latest/manual/cmake-buildsystem.7.html#build-configurations)
would be a good place to start.
