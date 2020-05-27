#
# Hoel library
#
# Makefile used to build the tests
#
# Public domain, no copyright. Use at your own risk
#

HOEL_INCLUDE=../include
CC=gcc
CFLAGS=-Wall -I$(HOEL_INCLUDE) -D_REENTRANT -DDEBUG -g -O0
HOEL_LOCATION=../src
LIBS=-lc -lyder -lhoel -ljansson -lcheck -lpthread -lm -lrt -lsubunit -L$(HOEL_LOCATION)

all: test

clean:
	rm -f *.o core multi valgrind.txt

libhoel.so:
	cd $(HOEL_LOCATION) && $(MAKE) debug

core: core.c
	$(CC) $(CFLAGS) core.c -o core $(LIBS)

test_core: libhoel.so core
	-LD_LIBRARY_PATH=$(HOEL_LOCATION):${LD_LIBRARY_PATH} ./core

multi: multi.c
	$(CC) $(CFLAGS) multi.c -o multi $(LIBS)

test_multi: libhoel.so multi
	-LD_LIBRARY_PATH=$(HOEL_LOCATION):${LD_LIBRARY_PATH} ./multi

test: test_core
