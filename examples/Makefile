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
CFLAGS=-c -Wall -D_REENTRANT $(ADDITIONALFLAGS)
HOEL_LOCATION=../src
LIBS=-lc -ldl -lhoel -L$(HOEL_LOCATION)
ADDITIONALFLAGS=-O3

all: example_sqlite3 example_mysql example_pgsql

clean:
	rm -f example_sqlite3 example_mysql example_pgsql

debug: ADDITIONALFLAGS=-DDEBUG -g -O0

debug: example_sqlite3 example_mysql example_pgsql

libhoel.so:
	cd $(HOEL_LOCATION) && $(MAKE) debug

example_sqlite3: libhoel.so example_sqlite3.c
	$(CC) -o example_sqlite3 example_sqlite3.c $(ADDITIONALFLAGS) $(LIBS)

example_mysql: libhoel.so example_mysql.c
	$(CC) -o example_mysql example_mysql.c $(ADDITIONALFLAGS) $(LIBS)

example_pgsql: libhoel.so example_pgsql.c
	$(CC) -o example_pgsql example_pgsql.c $(ADDITIONALFLAGS) $(LIBS)

test: example_sqlite3 example_mysql example_pgsql
	LD_LIBRARY_PATH=$(HOEL_LOCATION):${LD_LIBRARY_PATH} ./example_pgsql
	LD_LIBRARY_PATH=$(HOEL_LOCATION):${LD_LIBRARY_PATH} ./example_mysql
	LD_LIBRARY_PATH=$(HOEL_LOCATION):${LD_LIBRARY_PATH} ./example_sqlite3
