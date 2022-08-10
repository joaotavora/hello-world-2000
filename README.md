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

1. [Bare bones setup](#bare-bones)
2. [Top-level Makefile](#makefile)
3. [A library](#a-library)
4. [`catch2` tests](#catch2-tests)
5. [Google benchmark](#google-benchmark)

<a name="bare-bones"></a>
## Bare bones

By "bare bones" we mean a project setup with the fewest frills or
dependencies that is still conceivably useful for routine development
of a C++ library or program.

### Make four source files

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

### Make a build subdirectory

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

### Tell Conan to install packages, maybe build them

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

### In the build subdir, tell CMake to generate build files

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

### Almost Done! Now just type `make`

Makefiles were generated, because the default CMake generator is
`-G"Unix Makefiles"`.  It could be `-GNinja` if one has
[Ninja](https://ninja-build.org/) installed.

```sh
# inside hello-world/build
make
```

### Run this masterpiece of program

```sh
# inside hello-world/build
bin/hello madarfacar
{"Hello":"World","args":["bin/hello","madarfacar"]}
```

### Enable use of a LSP language server like `clangd`

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

### How did `make` compile my program?

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

<a name="build-release"></a>
### Make a Release build with -O3 and the NDEBUG preprocessor define

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

<a name="build-debug"></a>
### Make a Debug build with -O0 and the NDEBUG preprocessor define

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


## Top-level Makefile

To make the working with the bare-bones setup a little more
confortable, we're adding a single new source file, a top-level
`Makefile` right besides our `CMakeLists.txt`.  [Here it
is][top-level-makefile].

Don't confuse the top-level `Makefile` with any `build-*/Makefile`
files generated by CMake.  The top-level `Makefile` doesn't know
anything about the C++ project structure: that's CMake's job.  In fact
CMake could just just as well generate, say, `ninja.build` files.

We are going use the top-level `Makefile` to keep useful scripts that
do the steps we did manually in the [preceding section](#bare-bones).

These scripts _could_ be housed in separate files in a `tools/`
subdirectory.  But using a single file is easier here and it allows
for code to be shared between scripts.

Here are some of its useful targets:

* `build-release` and `build-debug`: Invoke CMake to create the two
  directories described in the [previous](#build-release) two
  [sections](#build-debug).  Doesn't do any building.

  In fact, these targets are actually pattern-rules of the form
  `build-%`, meaning it's reasonably easy to tweak the Makefile to add
  a new configurations with different defines.
  
* `all`: The default target.  Actually builds the program in the
  previously created `build-release` and `build-debug` targets.
  
* `watch-release` and `watch-debug`: Uses the [`entr`][entr] program
  to continuously monitor for changes in the `src/*` hierarchy, build
  the project and run it.  Depends on `build-release`/`build-debug`.

* `compile_commands.json`: Links in a top-level
  `compile_commands.json` by first calling one of `build-*` targets.
  
* `clean`: cleans up any temporary files.  Similar to `git clean
  -fdx`, but not as aggressive.

[top-level-makefile]: https://github.com/joaotavora/hello-world-2000/blob/master/Makefile
[entr]: https://eradman.com/entrproject/

<a name=a-library></a>
## A library

A useful complication to introduce at this point is a library.  Having
part of the source code compile as a library makes it possible to:

1. Suggest/enforce good API separation between services offered by the
   library and how to make use of those services in programs;

2. Write functional tests and benchmarks in a C++ framework that link
   against the library and directly exercise that API;

3. Eventually distribute our code as a library so that others may link
   (statically or dynamically) against it in their programs.
   
For now, we're going to create an actual static library object.  In
future installments we could create a header-only library or a shared
library object.

### Rearrange the C++ sources 

The first thing to do is to shuffle our sources a bit.  This is what
the directory structure should look like:

```
❯ tree src
src/
|-- core/
|   |-- hello.cpp
|   `-- hello.h
`-- main.cpp
```

And here's the full content of those 3 files:

```c++
// src/core/hello.h
#include <span>
#include <string>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

json greet(std::span<std::string> args);

// src/core/hello.cpp
#include "hello.h"

json greet(std::span<std::string> args) {
  return json{{"Hello", "World"}, {"args", args}};
}

// src/main.cpp
#include <iostream>
#include <vector>
#include <string>

#include "core/hello.h"

int main(int argc, char* argv[]) {
  std::vector<std::string> args(argv, argv + argc);
  auto j = greet(args);
  std::cout << j << "\n";
}
```

### Tweak `CMakeLists.txt`

In `CMakeLists.txt`, replace the previous `add_executable` block with
something slightly beefier:

```CMake
...

# Add a fancy-sounding "core lib"
set(core_lib ${PROJECT_NAME}-lib)
file(GLOB_RECURSE src_cpp CONFIGURE_DEPENDS "src/core/*.cpp")
add_library(${core_lib} STATIC ${src_cpp}) 
target_link_libraries(${core_lib} PRIVATE ${CONAN_LIBS})
target_include_directories(${core_lib} PUBLIC ./src)
set_target_properties(${core_lib} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

# The silly example -DHELLO_IS_HELLO preprocessor directive can still apply
target_compile_definitions(${core_lib} PUBLIC HELLO_IS_HELLO)

# Now make the "hello" executable target depending on "core lib"
file(GLOB src_cpp CONFIGURE_DEPENDS "src/*.cpp")
add_executable(${PROJECT_NAME} ${src_cpp})
target_link_libraries(${PROJECT_NAME} PRIVATE ${core_lib} ${CONAN_LIBS})
```

### Try it out (and understand what happened)

We can take advantage of the Makefile [created previously](#makefile):

```
$ make clean debug
```

This re-creates the `build-debug` directory with the new setup and then
re-builds the project for the "debug" configuration.  

We can see that we have kept the `build-debug/bin/hello` program,
which functions as before, but also gained a new
`build-debug/lib/libhello.a` file.

If one runs the above command as `VERBOSE=1 make clean debug`, the
command invocations pertaining to library creation become evident:

```
make[3]: Entering directory '../build-debug'
...
/usr/bin/ar qc lib/libhello.a "CMakeFiles/hello-lib.dir/src/core/hello.cpp.o"
/usr/bin/ranlib lib/libhello.a
```

As can be seen, the `ar` program is first run to create the
`libhello.a` file, which is a standard name for a libray that is
simply an archive of object files.  Then the `ranlib` program runs to
add an index to this archive.

The `build-debug/bin/hello` program is created later as the project of
mashing together the translation unit of `src/main.cpp` and the
archive file created above.

```
make[3]: Entering directory '.../build-debug'
...
/usr/bin/clang++ -gdwarf-4 -fsanitize=address -fsanitize=undefined     \
                 -g  CMakeFiles/hello.dir/src/main.cpp.o -o bin/hello  \
                 lib/libhello.a
```

<a name=catch2-tests></a>
## `catch2` tests

_**TODO**: very incomplete_


### Setup

`CMakeLists.txt`

```Cmake
...
# Make the "hello-tests" executable target
set(tests_exec ${PROJECT_NAME}-tests)
file(GLOB_RECURSE src_cpp CONFIGURE_DEPENDS "test/*.cpp")
add_executable(${tests_exec} ${src_cpp})
target_link_libraries(${tests_exec} PRIVATE ${core_lib} ${CONAN_LIBS})
```

`conanfile.txt`

```Conanfile
[requires]
nlohmann_json/3.10.5
catch2/3.1.0

[generators]
cmake
```

### Actual tests

`test/main.cpp` contains the actual tests and the `main` function.
They could be in separate files, the CMake code glob would do the
right thing.

```
#include <vector>
#include <string>

#define CATCH_CONFIG_RUNNER
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_session.hpp>

#include "core/hello.h"

TEST_CASE("Greeting is acceptable", "[core]") {
  auto args = std::vector<std::string>{"foo", "bar"};
  auto g = greet(args);
  REQUIRE(g.contains("Hello"));
  REQUIRE(g.at("Hello") == "World");
  REQUIRE(g.contains("args"));
  REQUIRE(g.at("args").is_array());
  REQUIRE(g.at("args").at(0) == "foo");
  REQUIRE(g.at("args").at(1) == "bar");
}

int main(int argc, char** argv)
{
  int result = Catch::Session().run( argc, argv );
  if (result != 0) return result;
}
```

### Makefile tricks

`Makefile`

```Make
...
watch-%:
	rg --files src test | entr -r -s 'make check-$*'

check-%: %
	./build-$*/bin/hello-tests
...
```

### Try it out

```
make check-debug
```

or better yet

```
make watch-debug
```


<a name=google-benchmark></a>
## Google benchmark

_**TODO**: very incomplete_
