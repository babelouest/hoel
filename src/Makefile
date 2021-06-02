#
# Hoel Framework
#
# Makefile used to build the software
#
# Copyright 2014-2020 Nicolas Mora <mail@babelouest.org>
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

PROJECT_NAME=hoel
PROJECT_DESCRIPTION=C Database abstraction library with json based language
PROJECT_BUGREPORT_PATH=https://github.com/babelouest/hoel/issues
PKGCONF_REQ=
PKGCONF_REQ_PRIVATE=
HOEL_INCLUDE=../include
DESTDIR=/usr/local
CONFIG_TEMPLATE=$(HOEL_INCLUDE)/hoel-cfg.h.in
CONFIG_FILE=$(HOEL_INCLUDE)/hoel-cfg.h
PKGCONFIG_FILE=../libhoel.pc
PKGCONFIG_TEMPLATE=../libhoel.pc.in
CFLAGS+=-c -fPIC -Wall -Werror -Wextra -I$(HOEL_INCLUDE) $(FLAGS_MARIADB) $(FLAGS_PGSQL) -D_REENTRANT $(ADDITIONALFLAGS) $(CPPFLAGS)
LIBS=-L$(DESTDIR)/lib -lc -ljansson -lyder -lorcania $(LIBS_SQLITE) $(LIBS_PGSQL) $(LIBS_MARIADB)
OBJECTS=hoel-sqlite.o hoel-mariadb.o hoel-pgsql.o hoel-simple-json.o hoel.o
OUTPUT=libhoel.so
VERSION_MAJOR=1
VERSION_MINOR=4
VERSION_PATCH=18

all: release

$(CONFIG_FILE):
	@cp $(CONFIG_TEMPLATE) $(CONFIG_FILE)
	@echo Config file $(CONFIG_FILE) generated
	@sed -i -e 's/$${PROJECT_VERSION}/$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)/g' $(CONFIG_FILE)        
	@sed -i -e 's/$${PROJECT_VERSION_MAJOR}/$(VERSION_MAJOR)/g' $(CONFIG_FILE)        
	@sed -i -e 's/$${PROJECT_VERSION_MINOR}/$(VERSION_MINOR)/g' $(CONFIG_FILE)        
	@sed -i -e 's/$${PROJECT_VERSION_PATCH}/$(VERSION_PATCH)/g' $(CONFIG_FILE)        
	@sed -i -e 's/$${PROJECT_VERSION_NUMBER}/$(shell printf '%02d' $(VERSION_MAJOR))$(shell printf '%02d' $(VERSION_MINOR))$(shell printf '%02d' $(VERSION_PATCH))/g' $(CONFIG_FILE)        
	@if [ "$(ENABLE_SQLITE)" = "1" ]; then \
		sed -i -e 's/\#cmakedefine _HOEL_SQLITE/\#define _HOEL_SQLITE/g' $(CONFIG_FILE); \
		echo "SQLITE SUPPORT  ENABLED"; \
	else \
		sed -i -e 's/\#cmakedefine _HOEL_SQLITE/\/* #undef _HOEL_SQLITE *\//g' $(CONFIG_FILE); \
		echo "SQLITE SUPPORT  DISABLED"; \
	fi
	@if [ "$(ENABLE_MARIADB)" = "1" ]; then \
		sed -i -e 's/\#cmakedefine _HOEL_MARIADB/\#define _HOEL_MARIADB/g' $(CONFIG_FILE); \
		echo "MARIADB SUPPORT ENABLED"; \
	else \
		sed -i -e 's/\#cmakedefine _HOEL_MARIADB/\/* #undef _HOEL_MARIADB *\//g' $(CONFIG_FILE); \
		echo "MARIADB SUPPORT DISABLED"; \
	fi
	@if [ "$(ENABLE_PGSQL)" = "1" ]; then \
		sed -i -e 's/\#cmakedefine _HOEL_PGSQL/\#define _HOEL_PGSQL/g' $(CONFIG_FILE); \
		echo "PGSQL SUPPORT   ENABLED"; \
	else \
		sed -i -e 's/\#cmakedefine _HOEL_PGSQL/\/* #undef _HOEL_PGSQL *\//g' $(CONFIG_FILE); \
		echo "PGSQL SUPPORT   DISABLED"; \
	fi

$(PKGCONFIG_FILE):
	@cp $(PKGCONFIG_TEMPLATE) $(PKGCONFIG_FILE)
	@echo Pkgconfig file $(PKGCONFIG_FILE) generated
	@sed -i -e 's#@CMAKE_INSTALL_PREFIX@#$(DESTDIR)#g' $(PKGCONFIG_FILE)
	@sed -i -e 's/@CMAKE_INSTALL_LIBDIR@/lib/g' $(PKGCONFIG_FILE)
	@sed -i -e 's/@CMAKE_INSTALL_INCLUDEDIR@/include/g' $(PKGCONFIG_FILE)
	@sed -i -e 's/@PROJECT_NAME@/$(PROJECT_NAME)/g' $(PKGCONFIG_FILE)
	@sed -i -e 's/@PROJECT_DESCRIPTION@/$(PROJECT_DESCRIPTION)/g' $(PKGCONFIG_FILE)
	@sed -i -e 's|@PROJECT_BUGREPORT_PATH@|$(PROJECT_BUGREPORT_PATH)|g' $(PKGCONFIG_FILE)
	@sed -i -e 's/@LIBRARY_VERSION@/$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)/g' $(PKGCONFIG_FILE)
	@sed -i -e 's/@PKGCONF_REQ@/$(PKGCONF_REQ)/g' $(PKGCONFIG_FILE)
	@sed -i -e 's/@PKGCONF_REQ_PRIVATE@/$(PKGCONF_REQ_PRIVATE)/g' $(PKGCONFIG_FILE)

libhoel.so: $(OBJECTS)
	$(CC) -shared -fPIC -Wl,-soname,$(OUTPUT) -o $(OUTPUT).$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH) $(OBJECTS) $(LIBS) $(LDFLAGS)
	ln -sf $(OUTPUT).$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH) $(OUTPUT)

libhoel.a: $(OBJECTS)
	ar rcs libhoel.a $(OBJECTS)

%.o: %.c $(CONFIG_FILE) $(HOEL_INCLUDE)/hoel.h $(HOEL_INCLUDE)/h-private.h
	$(CC) $(CFLAGS) $<

clean:
	rm -f *.o *.so *.a $(OUTPUT) $(OUTPUT).* $(CONFIG_FILE) $(PKGCONFIG_FILE)

install: all $(PKGCONFIG_FILE)
	cp $(OUTPUT).$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH) $(DESTDIR)/lib
	mkdir -p $(DESTDIR)/lib/pkgconfig/ $(DESTDIR)/include
	install -m644 $(PKGCONFIG_FILE) $(DESTDIR)/lib/pkgconfig/
	install -m644 $(HOEL_INCLUDE)/hoel.h $(DESTDIR)/include
	install -m644 $(CONFIG_FILE) $(DESTDIR)/include
	-ldconfig $(DESTDIR)/lib

static-install: static
	cp libhoel.a $(DESTDIR)/lib
	mkdir -p $(DESTDIR)/lib/pkgconfig/ $(DESTDIR)/include
	install -m644 $(PKGCONFIG_FILE) $(DESTDIR)/lib/pkgconfig
	install -m644 $(HOEL_INCLUDE)/hoel.h $(DESTDIR)/include
	install -m644 $(CONFIG_FILE) $(DESTDIR)/include

uninstall:
	rm -f $(DESTDIR)/lib/$(OUTPUT) $(DESTDIR)/lib/libhoel.a
	rm -f $(DESTDIR)/lib/$(OUTPUT).*
	rm -f $(DESTDIR)/include/hoel.h
	rm -f $(DESTDIR)/include/$(CONFIG_FILE)
	rm -f $(DESTDIR)/lib/pkgconfig/$(PKGCONFIG_FILE)

debug: ADDITIONALFLAGS=-DDEBUG -g -O0

debug: libhoel.so

release: ADDITIONALFLAGS=-O3

release: libhoel.so

static-debug: ADDITIONALFLAGS=-DDEBUG -g -O0

static-debug: libhoel.a

static: ADDITIONALFLAGS=-O3

static: libhoel.a
