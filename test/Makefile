#
# Hoel library
#
# Makefile used to build the tests
#
# Public domain, no copyright. Use at your own risk
#

HOEL_INCLUDE=../include
HOEL_LOCATION=../src
HOEL_LIBRARY=$(HOEL_LOCATION)/libhoel.so
CC=gcc
CFLAGS=-Wall -Werror -Wextra -I$(HOEL_INCLUDE) -D_REENTRANT -DDEBUG -g -O0
HOEL_DB_TEST=/tmp/test.db
LDFLAGS=-lc $(shell pkg-config --libs liborcania) $(shell pkg-config --libs libyder) $(shell pkg-config --libs libhoel) $(shell pkg-config --libs jansson) -L$(HOEL_LIBRARY) $(shell pkg-config --libs check)
VALGRIND_COMMAND=valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all
TARGET=core multi
VERBOSE=0
MEMCHECK=0

all: test

clean:
	rm -f *.o $(TARGET) $(HOEL_DB_TEST) valgrind-*.txt *.log

$(HOEL_DB_TEST):
	sqlite3 $(HOEL_DB_TEST) < test.sqlite3.sql

$(HOEL_LIBRARY): $(HOEL_INCLUDE)/hoel.h $(HOEL_LOCATION)/hoel.c $(HOEL_LOCATION)/hoel-mariadb.c $(HOEL_LOCATION)/hoel-pgsql.c $(HOEL_LOCATION)/hoel-sqlite.c $(HOEL_LOCATION)/hoel-simple-json.c
	cd $(HOEL_LOCATION) && $(MAKE) debug

%: $(HOEL_LIBRARY) %.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

test_%: % $(HOEL_LIBRARY) $(HOEL_DB_TEST)
	@if [ "$(VERBOSE)" = "0" ] && [ "$(MEMCHECK)" = "0" ]; then \
		LD_LIBRARY_PATH=$(HOEL_LOCATION):${LD_LIBRARY_PATH} ./run_test.sh ./$^; \
	elif [ "$(MEMCHECK)" = "0" ]; then \
		LD_LIBRARY_PATH=$(HOEL_LOCATION):${LD_LIBRARY_PATH} ./$^; \
	else \
		CK_FORK=no LD_LIBRARY_PATH=$(HOEL_LOCATION):${LD_LIBRARY_PATH} $(VALGRIND_COMMAND) ./$^ 2>valgrind-$@.txt; \
	fi

test: $(TARGET) test_core test_multi

check: test
