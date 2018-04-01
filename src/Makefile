#
# Hoel Framework
#
# Makefile used to build the software
#
# Copyright 2014-2018 Nicolas Mora <mail@babelouest.org>
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

ifndef DISABLE_SQLITE
FLAGS_SQLITE=-D_HOEL_SQLITE
LIBS_SQLITE=-lsqlite3
endif

ifndef DISABLE_MARIADB
FLAGS_MARIADB=-D_HOEL_MARIADB $(shell mysql_config --cflags)
LIBS_MARIADB=$(shell mysql_config --libs_r)
endif

ifndef DISABLE_POSTGRESQL
FLAGS_PGSQL=-D_HOEL_PGSQL -I/usr/include/postgresql/
LIBS_PGSQL=-lpq
endif

DESTDIR=/usr/local
CFLAGS+=-c -fPIC -Wall -I$(DESTDIR)/include $(FLAGS_SQLITE) $(FLAGS_MARIADB) $(FLAGS_PGSQL) -D_REENTRANT $(ADDITIONALFLAGS) $(CPPFLAGS)
LIBS=-L$(DESTDIR)/lib -lc -ljansson -lyder -lorcania $(LIBS_SQLITE) $(LIBS_PGSQL) $(LIBS_MARIADB)
OUTPUT=libhoel.so
VERSION=1.4.1

all: release

hoel.o: hoel.c ../include/hoel.h
	$(CC) $(CFLAGS) hoel.c

hoel-sqlite.o: hoel-sqlite.c ../include/hoel.h
	$(CC) $(CFLAGS) hoel-sqlite.c

hoel-mariadb.o: hoel-mariadb.c ../include/hoel.h
	$(CC) $(CFLAGS) hoel-mariadb.c

hoel-pgsql.o: hoel-pgsql.c ../include/hoel.h
	$(CC) $(CFLAGS) hoel-pgsql.c

hoel-simple-json.o: hoel-simple-json.c ../include/hoel.h
	$(CC) $(CFLAGS) hoel-simple-json.c

libhoel.so: hoel-sqlite.o hoel-mariadb.o hoel-pgsql.o hoel-simple-json.o hoel.o
	$(CC) -shared -Wl,-soname,$(OUTPUT).$(VERSION) -o $(OUTPUT).$(VERSION) hoel-sqlite.o hoel-mariadb.o hoel-pgsql.o hoel-simple-json.o hoel.o $(LIBS) $(LDFLAGS)
	ln -sf $(OUTPUT).$(VERSION) $(OUTPUT)

libhoel.a: hoel-sqlite.o hoel-mariadb.o hoel-pgsql.o hoel-simple-json.o hoel.o
	ar rcs libhoel.a hoel-sqlite.o hoel-mariadb.o hoel-pgsql.o hoel-simple-json.o hoel.o

clean:
	rm -f *.o *.so *.a $(OUTPUT) $(OUTPUT).*

install: all
	cp $(OUTPUT).$(VERSION) $(DESTDIR)/lib
	cp ../include/hoel.h $(DESTDIR)/include
	/sbin/ldconfig

static-install: static
	cp libhoel.a $(DESTDIR)/lib
	cp ../include/hoel.h $(DESTDIR)/include

uninstall:
	rm -f $(DESTDIR)/lib/$(OUTPUT) $(DESTDIR)/lib/libhoel.a
	rm -f $(DESTDIR)/lib/$(OUTPUT).*
	rm -f $(DESTDIR)/include/hoel.h

debug: ADDITIONALFLAGS=-DDEBUG -g -O0

debug: libhoel.so

release: ADDITIONALFLAGS=-O3

release: libhoel.so

static-debug: ADDITIONALFLAGS=-DDEBUG -g -O0

static-debug: libhoel.a

static: ADDITIONALFLAGS=-O3

static: libhoel.a
