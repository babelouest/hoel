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
ENABLE_SQLITE=1
LIBS_SQLITE=-lsqlite3
else
ENABLE_SQLITE=0
endif

ifndef DISABLE_MARIADB
ENABLE_MARIADB=1
FLAGS_MARIADB=$(shell mysql_config --cflags)
LIBS_MARIADB=$(shell mysql_config --libs_r)
else
ENABLE_MARIADB=0
endif

ifndef DISABLE_POSTGRESQL
ENABLE_PGSQL=1
FLAGS_PGSQL=$(shell pkg-config --cflags libpq)
LIBS_PGSQL=$(shell pkg-config --libs libpq)
else
ENABLE_PGSQL=0
endif

HOEL_INCLUDE=../include
DESTDIR=/usr/local
CONFIG_TEMPLATE=$(HOEL_INCLUDE)/hoel-cfg.h.in
CONFIG_FILE=$(HOEL_INCLUDE)/hoel-cfg.h
CFLAGS+=-c -fPIC -Wall -I$(HOEL_INCLUDE) $(FLAGS_MARIADB) $(FLAGS_PGSQL) -D_REENTRANT $(ADDITIONALFLAGS) $(CPPFLAGS)
LIBS=-L$(DESTDIR)/lib -lc -ljansson -lyder -lorcania $(LIBS_SQLITE) $(LIBS_PGSQL) $(LIBS_MARIADB)
OBJECTS=hoel-sqlite.o hoel-mariadb.o hoel-pgsql.o hoel-simple-json.o hoel.o
OUTPUT=libhoel.so
VERSION=1.4.6

all: release

$(CONFIG_FILE):
	@cp $(CONFIG_TEMPLATE) $(CONFIG_FILE)
	@echo Config file $(CONFIG_FILE) generated
	@sed -i -e 's/$${PROJECT_VERSION}/$(VERSION)/g' $(CONFIG_FILE)        
	@if [ "$(ENABLE_SQLITE)" = "1" ]; then \
		sed -i -e 's/\#cmakedefine _HOEL_SQLITE/\#define _HOEL_SQLITE/g' $(CONFIG_FILE); \
		echo "SQLITE SUPPORT  DISABLED"; \
	else \
		sed -i -e 's/\#cmakedefine _HOEL_SQLITE/\/* #undef _HOEL_SQLITE *\//g' $(CONFIG_FILE); \
		echo "SQLITE SUPPORT  ENABLED"; \
	fi
	@if [ "$(ENABLE_MARIADB)" = "1" ]; then \
		sed -i -e 's/\#cmakedefine _HOEL_MARIADB/\#define _HOEL_MARIADB/g' $(CONFIG_FILE); \
		echo "MARIADB SUPPORT DISABLED"; \
	else \
		sed -i -e 's/\#cmakedefine _HOEL_MARIADB/\/* #undef _HOEL_MARIADB *\//g' $(CONFIG_FILE); \
		echo "MARIADB SUPPORT ENABLED"; \
	fi
	@if [ "$(ENABLE_PGSQL)" = "1" ]; then \
		sed -i -e 's/\#cmakedefine _HOEL_PGSQL/\#define _HOEL_PGSQL/g' $(CONFIG_FILE); \
		echo "PGSQL SUPPORT   DISABLED"; \
	else \
		sed -i -e 's/\#cmakedefine _HOEL_PGSQL/\/* #undef _HOEL_PGSQL *\//g' $(CONFIG_FILE); \
		echo "PGSQL SUPPORT   ENABLED"; \
	fi

libhoel.so: $(CONFIG_FILE) $(OBJECTS)
	$(CC) -shared -fPIC -Wl,-soname,$(OUTPUT) -o $(OUTPUT).$(VERSION) $(OBJECTS) $(LIBS) $(LDFLAGS)
	ln -sf $(OUTPUT).$(VERSION) $(OUTPUT)

libhoel.a: $(CONFIG_FILE) $(OBJECTS)
	ar rcs libhoel.a $(OBJECTS)

%.o: %.c $(CONFIG_FILE) $(HOEL_INCLUDE)/hoel.h $(HOEL_INCLUDE)/h-private.h
	$(CC) $(CFLAGS) $<

clean:
	rm -f *.o *.so *.a $(OUTPUT) $(OUTPUT).* $(CONFIG_FILE)

install: all
	cp $(OUTPUT).$(VERSION) $(DESTDIR)/lib
	install -m644 $(HOEL_INCLUDE)/hoel.h $(DESTDIR)/include
	install -m644 $(CONFIG_FILE) $(DESTDIR)/include
	/sbin/ldconfig

static-install: static
	cp libhoel.a $(DESTDIR)/lib
	install -m644 $(HOEL_INCLUDE)/hoel.h $(DESTDIR)/include
	install -m644 $(CONFIG_FILE) $(DESTDIR)/include

uninstall:
	rm -f $(DESTDIR)/lib/$(OUTPUT) $(DESTDIR)/lib/libhoel.a
	rm -f $(DESTDIR)/lib/$(OUTPUT).*
	rm -f $(DESTDIR)/include/hoel.h
	rm -f $(DESTDIR)/include/hoel-cfg.h

debug: ADDITIONALFLAGS=-DDEBUG -g -O0

debug: libhoel.so

release: ADDITIONALFLAGS=-O3

release: libhoel.so

static-debug: ADDITIONALFLAGS=-DDEBUG -g -O0

static-debug: libhoel.a

static: ADDITIONALFLAGS=-O3

static: libhoel.a
