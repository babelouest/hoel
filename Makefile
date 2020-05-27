#
# Example program
#
# Makefile used to build all programs
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

LIBHOEL_LOCATION=./src
EXAMPLE_LOCATION=./examples
TEST_LOCATION=./test

all: release

debug:
	cd $(LIBHOEL_LOCATION) && $(MAKE) debug $*

install:
	cd $(EXAMPLE_LOCATION) && $(MAKE) install $*

clean:
	cd $(LIBHOEL_LOCATION) && $(MAKE) clean
	cd $(EXAMPLE_LOCATION) && $(MAKE) clean
	cd $(TEST_LOCATION) && $(MAKE) clean

release:
	cd $(LIBHOEL_LOCATION) && $(MAKE) $*

simple_example:
	cd $(EXAMPLE_LOCATION) && $(MAKE) $*

example_sqlite3:
	cd $(EXAMPLE_LOCATION) && $(MAKE) example_sqlite3 $*

example_mariadb:
	cd $(EXAMPLE_LOCATION) && $(MAKE) example_mariadb $*

example_pgsql:
	cd $(EXAMPLE_LOCATION) && $(MAKE) example_pgsql $*

check:
	cd $(TEST_LOCATION) && $(MAKE) test $*

doxygen:
	doxygen doc/doxygen.cfg
