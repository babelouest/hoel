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
LDFLAGS=-lc $(shell pkg-config --libs liborcania) $(shell pkg-config --libs libyder) $(shell pkg-config --libs libhoel) $(shell pkg-config --libs jansson) -L$(HOEL_LOCATION) $(shell pkg-config --libs check)
VALGRIND_COMMAND=valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all
TARGET=core multi
VERBOSE=0
MEMCHECK=0

all: test

clean:
	rm -f *.o $(TARGET) valgrind-*.txt *.log

$(HOEL_LIBRARY): $(HOEL_INCLUDE)/iddawc.h $(HOEL_LOCATION)/iddawc.c
	cd $(HOEL_LOCATION) && $(MAKE) debug

%: $(HOEL_LIBRARY) %.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

test_%: % $(HOEL_LIBRARY)
	@if [ "$(VERBOSE)" = "0" ] && [ "$(MEMCHECK)" = "0" ]; then \
		LD_LIBRARY_PATH=$(HOEL_LOCATION):${LD_LIBRARY_PATH} ./run_test.sh ./$^; \
	elif [ "$(MEMCHECK)" = "0" ]; then \
		LD_LIBRARY_PATH=$(HOEL_LOCATION):${LD_LIBRARY_PATH} ./$^ ; \
	else \
		CK_FORK=no LD_LIBRARY_PATH=$(HOEL_LOCATION):${LD_LIBRARY_PATH} $(VALGRIND_COMMAND) ./$^ 2>valgrind-$@.txt; \
	fi

test: $(TARGET) test_core test_multi

check: test
