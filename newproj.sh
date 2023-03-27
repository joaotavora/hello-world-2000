#!/bin/sh
git ls-files | xargs sed -i -e "s/hello/$1/g"
mv ./src/core/hello.h ./src/core/$1.h
mv ./src/core/hello.cpp ./src/core/$1.cpp

