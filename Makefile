#
# Example program
#
# Makefile used to build all programs
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

LIBHOEL_LOCATION=./src
EXAMPLE_LOCATION=./examples
TEST_LOCATION=./test

all: libhoel.so example_sqlite3 example_mariadb example_pgsql

debug:
	cd $(EXAMPLE_LOCATION) && $(MAKE) debug

clean:
	cd $(LIBHOEL_LOCATION) && $(MAKE) clean
	cd $(EXAMPLE_LOCATION) && $(MAKE) clean
	cd $(TEST_LOCATION) && $(MAKE) clean

libhoel.so:
	cd $(LIBHOEL_LOCATION) && $(MAKE)

simple_example:
	cd $(EXAMPLE_LOCATION) && $(MAKE)

example_sqlite3:
	cd $(EXAMPLE_LOCATION) && $(MAKE) example_sqlite3

example_mariadb:
	cd $(EXAMPLE_LOCATION) && $(MAKE) example_mariadb

example_pgsql:
	cd $(EXAMPLE_LOCATION) && $(MAKE) example_pgsql

test:
	cd $(TEST_LOCATION) && $(MAKE) test
