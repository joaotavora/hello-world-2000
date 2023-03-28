CXX=g++
CONAN_PROFILE=default
PROJECT_NAME=hello

all: release debug

build-debug: export CXXFLAGS := -gdwarf-4 -fsanitize=address -fsanitize=undefined
build-release: CMAKE_FLAGS=-DCMAKE_BUILD_TYPE=Release -G"Unix Makefiles"

build-%:
	mkdir -p build-$*
	(cd build-$* && conan install --build=missing --profile=${CONAN_PROFILE} ../) \
              || (ret=$$?; rm -rf $@ && exit $$ret)
	(cd build-$* && CXXFLAGS='${CXXFLAGS}' cmake ${CMAKE_FLAGS} ../)       \
              || (ret=$$?; rm -rf $@ && exit $$ret)
	ln -sf ./build-$*/compile_commands.json compile_commands.json

compile_commands.json: build-debug
	ln -sf build-debug/compile_commands.json compile_commands.json

watch-%:
	find src test -type f | entr -r -s 'make check-$*'

check-%: %
	./build-$*/bin/$(PROJECT_NAME)-tests

bench-%: %
	./build-$*/bin/$(PROJECT_NAME)-bench

demo-%: %
	./build-$*/bin/$(PROJECT_NAME)

%: build-%
	make -C build-$*

clean:
	rm -rf build-*

.PHONY: clean all watch
