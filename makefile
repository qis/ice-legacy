# Make
MAKEFLAGS += --no-print-directory

# Compiler
CC	:= clang
CXX	:= clang++

# Build Type
BUILD	?= Debug

# Project
PROJECT	:= $(shell grep "^project" CMakeLists.txt | cut -c9- | cut -d" " -f1)

# Targets
all: configure
	@cmake --build build

build:
	@mkdir build

configure: build
	cd build && CC="$(CC)" CXX="$(CXX)" cmake -DCMAKE_BUILD_TYPE=$(BUILD) -DCMAKE_INSTALL_PREFIX:PATH=.. ..

install: remove all
	@cmake --build build --target install

clean:
	rm -rf build

remove:
	rm -rf lib

format: configure
	@cmake --build build --target format
