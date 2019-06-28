
CC=gcc
CFLAGS=-fPIC -shared -g -Wall -D__LINUX__
LD_FLAGS=-ldl

.PHONY: all test
all: libwrap.so

OBJS: wrap.o os.o

libwrap.so: src/wrap.c os/linux/os.c 
	@echo "Building libwrap.so ..."
	$(CC) $(CFLAGS) -o ./lib/linux/$@ $^ $(LD_FLAGS)

clean:
	rm -f ./lib/linux/libwrap.so
	rm -f test/linux/test

test: test/test.c contrib/libyaml/src/.libs/libyaml.a test/scopecfgtest.c src/scopecfg.c
	@echo "Building Tests"
	$(CC) -Wall -g -o test/linux/test test/test.c
	$(CC) -Wall -g -o test/linux/scopecfgtest -I./src -I./contrib/libyaml/include test/scopecfgtest.c src/scopecfg.c contrib/libyaml/src/.libs/libyaml.a 
	test/linux/scopecfgtest

contrib/libyaml/src/.libs/libyaml.a:
	@echo "Building libyaml"
	cd ./contrib/libyaml && ./bootstrap
	cd ./contrib/libyaml && ./configure
	cd ./contrib/libyaml && make