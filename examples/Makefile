#
# Example program
#
# Makefile used to build the software
#
# Copyright 2014-2015 Nicolas Mora <mail@babelouest.org>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the MIT License
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU GENERAL PUBLIC LICENSE for more details.
#
CC=gcc
HOEL_INCLUDE=../include
CFLAGS=-c -Wall -I$(HOEL_INCLUDE) -D_REENTRANT -I$(PREFIX)/include $(ADDITIONALFLAGS)
HOEL_LOCATION=../src
LIBS=-lc -lhoel -ljansson -lyder -lorcania -L$(HOEL_LOCATION)
ADDITIONALFLAGS=-O3

all: libhoel.so example_sqlite3 example_mariadb example_pgsql example_mariadb_json

clean:
	rm -f *.o example_sqlite3 example_mariadb example_pgsql example_mariadb_json

debug: ADDITIONALFLAGS=-DDEBUG -g -O0

debug: libhoel.so example_sqlite3 example_mariadb example_pgsql example_mariadb_json

libhoel.so:
	cd $(HOEL_LOCATION) && $(MAKE) debug

example_sqlite3: example_sqlite3.c
	$(CC) -c $(CFLAGS) example_sqlite3.c
	$(CC) -o example_sqlite3 example_sqlite3.o $(LIBS)

example_mariadb: example_mariadb.c
	$(CC) -c $(CFLAGS) example_mariadb.c
	$(CC) -o example_mariadb example_mariadb.o $(LIBS)

example_pgsql: example_pgsql.c
	$(CC) -c $(CFLAGS) example_pgsql.c
	$(CC) -o example_pgsql example_pgsql.o $(LIBS)

example_mariadb_json: example_mariadb_json.c
	$(CC) -c $(CFLAGS) example_mariadb_json.c
	$(CC) -o example_mariadb_json example_mariadb_json.o $(LIBS)

test: test_pgsql test_mariadb test_sqlite3 test_mariadb_json

test_pgsql: example_pgsql
	LD_LIBRARY_PATH=$(HOEL_LOCATION):${LD_LIBRARY_PATH} ./example_pgsql

test_mariadb: example_mariadb
	LD_LIBRARY_PATH=$(HOEL_LOCATION):${LD_LIBRARY_PATH} ./example_mariadb

test_sqlite3: example_sqlite3
	LD_LIBRARY_PATH=$(HOEL_LOCATION):${LD_LIBRARY_PATH} ./example_sqlite3

test_mariadb_json: example_mariadb_json
	LD_LIBRARY_PATH=$(HOEL_LOCATION):${LD_LIBRARY_PATH} ./example_mariadb_json
