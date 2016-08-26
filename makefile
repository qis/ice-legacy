# Compiler
CC	:= gcc
CXX	:= g++

# Targets
all: configure
	@cmake --build build

build:
	@mkdir build

configure: build
	@cd build && CC="$(CC)" CXX="$(CXX)" cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX:PATH=.. ..

install: all
	@cmake --build build --target install

clean:
	rm -rf build
