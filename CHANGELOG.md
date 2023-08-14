# Hoel Changelog

## 1.4.28

- cmake: remove `DownloadProject` feature, now dependencies must be previously installed

## 1.4.27

- Add `h_build_where_clause`
- Build with flag `-Wconversion`

## 1.4.26

- Minor bugfixes

## 1.4.25

- Fix `CMAKE_MODULE_PATH` who was used as single value (for real)

## 1.4.24

- Fix pkg-config file with absolute path for `CMAKE_INSTALL_{INCLUDE,LIB}DIR`
- Fix `CMAKE_MODULE_PATH` who was used as single value

## 1.4.23

- Add missing `pthread_mutex_lock` for MariaDB and PostgreSQL in respective `h_last_insert_id`

## 1.4.22

- Fix `h_escape_string_mariadb`

## 1.4.21

- Replace `#define h_exec_query_sqlite` with a redefinition of `h_exec_query_sqlite` as a function
- Create or delete test db file for unit tests
- Small bugfixes or improvements

## 1.4.20

- Rename `h_exec_query_sqlite` to `h_execute_query_sqlite`, add `h_exec_query_sqlite` that redirects to `h_execute_query_sqlite`
- Implement all dummy functions when a database backend is not supported

## 1.4.19

- Use `o_strlen` instead of `strlen`

## 1.4.18

- Remove Travis CI tests

## 1.4.17

- Code cleaning
- Increase dependencies version required

## 1.4.16

- Use stdbool instead of `my_bool` which is deprecated
- Uppdate package dependencies in cmake script

## 1.4.15

- Fix memory leak
- Update doc

## 1.4.14

- Fix yet another set of bugs in the PostgreSQL backend
- Add multi-backend test suite

## 1.4.13

- Fix build when databases are disabled
- Fix ldconfig command in Makefile
- Add doxygen documentation

## 1.4.12

- Update SQlite `sqlite3_open_v2` optparameters to add `SQLITE_OPEN_FULLMUTEX` flag

## 1.4.11

- Add `h_escape_string_with_quotes` to help using escape strings with PostgreSQL
- Add mutex in pgsql connection because libpq isn't thread safe, like libmysql...
- Change struct _h_type_int value type from int to long long int

## 1.4.10

- Fix mysql package name dependency when build deb package

## 1.4.9

- Fix h_connect_pgsql to accept const char * parameter instead of char *
- Remove backquotes because Postgre seems not to like them
- minor bugfixes
- Install pkgconfig file when using Makefile

## 1.4.8

- Fix mariadb bug when empty column was returned as NULL

## 1.4.7

- Add support for binary columns in `struct _h_data` by adding length parameter in `struct _h_type_text`
- Code cleaning and bug fixes thanks to clang static analyzer

## 1.4.6

- CMake scripts improvements

## 1.4.5

- Add config file hoel-cfg.h dynamically built with the options
- Improve CI

## 1.4.4

- Add Travis CI
- Change cmake option BUILD_TESTING to BUILD_HOEL_TESTING
- Add RPM in CMake script package

## 1.4.3

- Fix memory leak
- Fix pkg-config information, add requires fields as mentionned in babelouest/ulfius#62
- Improve example_mariadb_json.c to make it more readable and understandable, clean code
- Removing the my_global.h include in hoel-mariadb.c
- Fix static library output file name babelouest/ulfius#55

## 1.4.2

- Fix Makefile soname

## 1.4.1

- Add Debian hardening patch on Makefile

## 1.4

- Add CMake installation script
- Various bugfixes
