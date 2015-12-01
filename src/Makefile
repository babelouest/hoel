#
# Ulfius Framework
#
# Makefile used to build the software
#
# Copyright 2014-2015 Nicolas Mora <mail@babelouest.org>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License
# as published by the Free Software Foundation;
# version 2.1 of the License.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU GENERAL PUBLIC LICENSE for more details.
#
# You should have received a copy of the GNU General Public
# License along with this library.	If not, see <http://www.gnu.org/licenses/>.
#
CC=gcc

# uncomment these lines if you want sqlite support
# HAS_SQLITE=-D_HOEL_SQLITE
# LIBS_SQLITE=-lsqlite3

# uncomment these lines if you want sqlite support
HAS_MARIADB=-D_HOEL_MARIADB
LIBS_MYSQL=-lmysqlclient

# uncomment these lines if you want sqlite support
# HAS_PGSQL=-D_HOEL_PGSQL
# LIBS_PGSQL=-lpq

CFLAGS=-c -fPIC -Wall $(HAS_SQLITE) $(HAS_MARIADB) $(HAS_PGSQL) -D_REENTRANT $(ADDITIONALFLAGS) -I/usr/include/postgresql/ -I/usr/include/mysql/
LIBS=-lc $(LIBS_SQLITE) $(LIBS_PGSQL) $(LIBS_MYSQL)
OUTPUT=libhoel.so
VERSION=0.7
PREFIX=/usr/local

all: release

hoel.o: hoel.c hoel.h
	$(CC) $(CFLAGS) hoel.c

libhoel.so: hoel.o
	$(CC) -shared -Wl,-soname,$(OUTPUT) -o $(OUTPUT).$(VERSION) hoel.o $(LIBS)
	ln -sf $(OUTPUT).$(VERSION) $(OUTPUT)

clean:
	rm -f *.o *.so $(OUTPUT) $(OUTPUT).$(VERSION)

install: all
	cp $(OUTPUT).$(VERSION) $(PREFIX)/lib
	cp hoel.h $(PREFIX)/include
	ldconfig

uninstall:
	rm -f $(PREFIX)/lib/$(OUTPUT)
	rm -f $(PREFIX)/lib/$(OUTPUT).$(VERSION)
	rm -f $(PREFIX)/include/hoel.h

debug: ADDITIONALFLAGS=-DDEBUG -g -O0

debug: libhoel.so

release: ADDITIONALFLAGS=-O3

release: libhoel.so
