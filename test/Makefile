#
# Hoel library
#
# Makefile used to build the tests
#
# Copyright 2017 Nicolas Mora <mail@babelouest.org>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU GENERAL PUBLIC LICENSE
# License as published by the Free Software Foundation;
# version 3 of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU GENERAL PUBLIC LICENSE for more details.
#
# You should have received a copy of the GNU General Public
# License along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

HOEL_INCLUDE=../include
CC=gcc
CFLAGS=-Wall -I$(HOEL_INCLUDE) -D_REENTRANT -DDEBUG -g -O0
HOEL_LOCATION=../src
LIBS=-lc -lyder -lhoel -ljansson -lcheck -lpthread -lm -lrt -lsubunit -L$(HOEL_LOCATION)

all: test

clean:
	rm -f *.o core valgrind.txt

libhoel.so:
	cd $(HOEL_LOCATION) && $(MAKE) debug

core: core.c
	$(CC) $(CFLAGS) core.c -o core $(LIBS)

test_core: libhoel.so core
	-LD_LIBRARY_PATH=$(HOEL_LOCATION):${LD_LIBRARY_PATH} ./core

test: test_core
