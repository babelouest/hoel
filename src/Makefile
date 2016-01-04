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
# FLAGS_SQLITE=-D_HOEL_SQLITE
# LIBS_SQLITE=-lsqlite3

# uncomment these lines if you want sqlite support
# FLAGS_MARIADB=-D_HOEL_MARIADB -I/usr/include/mysql/
# LIBS_MARIADB=-lmysqlclient

# uncomment these lines if you want sqlite support
# FLAGS_PGSQL=-D_HOEL_PGSQL -I/usr/include/postgresql/
# LIBS_PGSQL=-lpq

PREFIX=/usr/local
CFLAGS=-c -fPIC -Wall -I$(PREFIX)/include $(FLAGS_SQLITE) $(FLAGS_MARIADB) $(FLAGS_PGSQL) -D_REENTRANT $(ADDITIONALFLAGS)
LIBS=-L$(PREFIX)/lib -lc -ljansson -lyder $(LIBS_SQLITE) $(LIBS_PGSQL) $(LIBS_MARIADB)
OUTPUT=libhoel.so
VERSION=0.9

all: release

hoel.o: hoel.c hoel.h
	$(CC) $(CFLAGS) hoel.c

hoel-sqlite.o: hoel-sqlite.c hoel.h
	$(CC) $(CFLAGS) hoel-sqlite.c

hoel-mariadb.o: hoel-mariadb.c hoel.h
	$(CC) $(CFLAGS) hoel-mariadb.c

hoel-pgsql.o: hoel-pgsql.c hoel.h
	$(CC) $(CFLAGS) hoel-pgsql.c

hoel-simple-json.o: hoel-simple-json.c hoel.h
	$(CC) $(CFLAGS) hoel-simple-json.c

libhoel.so: hoel-sqlite.o hoel-mariadb.o hoel-pgsql.o hoel-simple-json.o hoel.o
	$(CC) -shared -Wl,-soname,$(OUTPUT) -o $(OUTPUT).$(VERSION) hoel-sqlite.o hoel-mariadb.o hoel-pgsql.o hoel-simple-json.o hoel.o $(LIBS)
	ln -sf $(OUTPUT).$(VERSION) $(OUTPUT)

clean:
	rm -f *.o *.so $(OUTPUT) $(OUTPUT).*

install: all
	cp $(OUTPUT).$(VERSION) $(PREFIX)/lib
	cp hoel.h $(PREFIX)/include
	ldconfig -r $(PREFIX)

uninstall:
	rm -f $(PREFIX)/lib/$(OUTPUT)
	rm -f $(PREFIX)/lib/$(OUTPUT).*
	rm -f $(PREFIX)/include/hoel.h

debug: ADDITIONALFLAGS=-DDEBUG -g -O0

debug: libhoel.so

release: ADDITIONALFLAGS=-O3

release: libhoel.so
