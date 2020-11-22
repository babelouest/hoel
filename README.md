# Hoel

[![Build Status](https://travis-ci.com/babelouest/hoel.svg?branch=master)](https://travis-ci.com/babelouest/hoel)
![.github/workflows/ccpp.yml](https://github.com/babelouest/hoel/workflows/.github/workflows/ccpp.yml/badge.svg)

Database abstraction library written in C.

Simple and easy to use database access library. Works with SQLite 3, MariaDB and PostgreSQL databases. Uses a JSON-based language with `jansson` to execute simple queries based on one table.

# Online documentation

See the [online documentation](https://babelouest.github.io/hoel/) for a doxygen format of the API documentation.

# Installation

## Prerequisites

### Jansson

Install [Jansson](http://www.digip.org/jansson/) library for JSON manipulation, minimum version 2.4. On a Debian-based platform, run the following command:

```shell
$ sudo apt-get install libjansson-dev
```

### Database libraries

Install Hoel database dependencies based on your requirements:
- SQLite3: Install the package `libsqlite3-dev`
- MariaDB: Install the package `libmariadb-dev`
- PostgreSQL: Install the package `libpq-dev`

### Distribution packages

[![Packaging status](https://repology.org/badge/vertical-allrepos/hoel.svg)](https://repology.org/metapackage/hoel)

Hoel is available in multiple distributions as official package. Check out your distribution documentation to install the package automatically.

```shell
$ # Example for Debian testing
$ sudo apt install libhoel-dev
```

### Pre-compiled packages

You can install Hoel with a pre-compiled package available in the [release pages](https://github.com/babelouest/hoel/releases/latest/). `jansson`, `sqlite3`, `libmariadb-client` and `libpq` development files packages are required to install Hoel. The packages files `hoel-dev-full_*` contain the libraries `orcania`, `yder` and `hoel`. It's also compiled with all the database backends.

For example, to install Hoel with the `hoel-dev-full_2.3.0_Debian_stretch_x86_64.tar.gz` package downloaded on the `releases` page, you must execute the following commands:

```shell
$ sudo apt install -y libjansson-dev libmariadb-dev libsqlite3-dev libpq-dev libsystemd-dev
$ wget https://github.com/babelouest/hoel/releases/download/v1.4.0/hoel-dev-full_1.4.0_Debian_stretch_x86_64.tar.gz
$ tar xf hoel-dev-full_1.4.0_Debian_stretch_x86_64.tar.gz
$ sudo dpkg -i liborcania-dev_1.2.0_Debian_stretch_x86_64.deb
$ sudo dpkg -i libyder-dev_1.2.0_Debian_stretch_x86_64.deb
$ sudo dpkg -i libhoel-dev_1.4.0_Debian_stretch_x86_64.deb
```

If there's no package available for your distribution, you can recompile it manually using `CMake` or `Makefile`.

## Install from the source

### CMake - Multi architecture

[CMake](https://cmake.org/download/) minimum 3.5 is required.

Run the CMake script in a sub-directory, example:

```shell
$ git clone https://github.com/babelouest/hoel.git
$ cd hoel/
$ mkdir build
$ cd build
$ cmake ..
$ make && sudo make install
```

The available options for CMake are:
- `-DWITH_SQLITE3=[on|off]` (default `on`): Enable/disable SQLite3 database backend
- `-DWITH_MARIADB=[on|off]` (default `on`): Enable/disable MariaDB database backend
- `-DWITH_PGSQL=[on|off]` (default `on`): Enable/disable PostgreSQL database backend
- `-DWITH_JOURNALD=[on|off]` (default `on`): Build with journald (SystemD) support for logging
- `-DBUILD_STATIC=[on|off]` (default `off`): Build the static archive in addition to the shared library
- `-DBUILD_HOEL_TESTING=[on|off]` (default `off`): Build unit tests
- `-DBUILD_HOEL_DOCUMENTATION=[on|off]` (default `off`): Build the documentation, doxygen is required
- `-DINSTALL_HEADER=[on|off]` (default `on`): Install header file `hoel.h`
- `-DBUILD_RPM=[on|off]` (default `off`): Build RPM package when running `make package`
- `-DCMAKE_BUILD_TYPE=[Debug|Release]` (default `Release`): Compile with debugging symbols or not

### Good ol' Makefile

Clone, compile and install [Orcania](https://github.com/babelouest/orcania) and [Yder](https://github.com/babelouest/yder) libraries.

### Orcania (Miscellaneous functions)

```shell
$ git clone https://github.com/babelouest/orcania.git
$ cd orcania/src
$ make && sudo make install
```

### Yder (simple logs library)

```shell
$ git clone https://github.com/babelouest/yder.git
$ cd yder/src
$ make
$ sudo make install
```

### Hoel

Download Hoel from GitHub repository.

```shell
$ git clone https://github.com/babelouest/hoel.git
$ cd hoel/src
$ make
$ sudo make install
```

By default, Hoel is compiled with the 3 databases support. If you don't need one or more database, follow these instructions

#### SQLite 3

Add DISABLE_SQLITE=1 to the `make` command:

```shell
$ cd hoel/src
$ make DISABLE_SQLITE=1
$ sudo make install
```

#### MariaDB

Add DISABLE_MARIADB=1 to the `make` command:

```shell
$ cd hoel/src
$ make DISABLE_MARIADB=1
$ sudo make install
```

#### Postgre SQL

Add DISABLE_POSTGRESQL=1 to the `make` command:

```shell
$ cd hoel/src
$ make DISABLE_POSTGRESQL=1
$ sudo make install
```

### Disable 2 backends

You can disable 2 databases backends to keep just one, simply add both parameters to the `make` command:

```shell
$ cd hoel/src
$ make DISABLE_MARIADB=1 DISABLE_POSTGRESQL=1
$ sudo make install
```

### Installation folder

By default, the shared library and the header file will be installed in the `/usr/local` location. To change this setting, you can modify the `DESTDIR` value in the `src/Makefile`.

# API Documentation

## Header files and compilation

To use Hoel in your code, include the file `hoel.h`.

```c
#include <hoel.h>
```

Use the flag `-lhoel` to include Hoel library in the linking process.

### Return values

When specified, some functions return `H_OK` on success, and other values otherwise. `H_OK` is 0, other values are non-0 values. The defined errors list is the following:

```c
#define H_OK                0  // No error
#define H_ERROR             1  // Generic error
#define H_ERROR_PARAMS      2  // Error in input parameters
#define H_ERROR_CONNECTION  3  // Error in database connection
#define H_ERROR_DISABLED    4  // Database connection is disabled
#define H_ERROR_QUERY       5  // Error executing query
#define H_ERROR_MEMORY      99 // Error allocating memory
```

### Memory allocation

Some function return allocated values. When the value is not a structure, you must use the function `h_free` to clean it. Otherwise, use the dedicated functions.

```c
/**
 * free data allocated by hoel functions
 */
void h_free(void * data);
```

### Initialization

To create a connection to a database, use its dedicated function

```c
/**
 * h_connect_sqlite
 * Opens a database connection to a sqlite3 db file
 * return pointer to a struct _h_connection * on sucess, NULL on error
 */
struct _h_connection * h_connect_sqlite(const char * db_path);

/**
 * h_connect_mariadb
 * Opens a database connection to a mariadb server
 * return pointer to a struct _h_connection * on success, NULL on error
 */
struct _h_connection * h_connect_mariadb(const char * host, const char * user, const char * passwd, const char * db, const unsigned int port, const char * unix_socket);

/**
 * h_connect_pgsql
 * Opens a database connection to a PostgreSQL server
 * return pointer to a struct _h_connection * on sucess, NULL on error
 */
struct _h_connection * h_connect_pgsql(char * conninfo);
```

All these functions return a struct _h_connection * on success. This pointer will be needed on every call to Hoel functions.

When you no longer need your connection, close it using the function `h_close_db`. This will close the connection to the database and free the memory allocated by the connection.

```c
/**
 * Close a database connection
 * return H_OK on success
 */
int h_close_db(struct _h_connection * conn);
```

The connection must be cleaned when it's no longer needed.

```c
/**
 * h_clean_connection
 * free memory allocated by the struct _h_connection
 * return H_OK on success
 */
int h_clean_connection(struct _h_connection * conn);
```

### Escape string

If you need to escape parameters, you can use the functions `h_escape_string`, the returned value must be h_free'd after use.

The meaning of existence of the function `h_escape_string_with_quotes` is because of PostgreSQL ways to escape an unsafe string.

TL;DR:
- Use `h_escape_string_with_quotes` only if you use a PostgreSQL database.
- You can use both `h_escape_string_with_quotes` and `h_escape_string` if you use only SQLite3 and/or MariaDB databases.

If the unsafe string contains backslashes `"\"`, the escaped string will have the pattern `E'<safe>'`, otherwise it will have the pattern `'<safe>'`. Since the function `h_escape_string` must return the safe string only, without surroundings quotes `'`, the format `E'<safe>'`  is incompatible with Hoel API.

Therefore, using `h_escape_string` with PostgreSQL could lead to undefined behaviour and more importantly exploitable bugs.

To avoid this, on a PostgreSQL database, an escaped string using `h_escape_string` that should return the format `E'<safe>'` will return `NULL`, and an escaped string using `h_escape_string_with_quotes` will return the format `E'<safe>'`.

```c
/**
 * h_escape_string
 * Escapes a string
 * returned value must be h_free'd after use
 */
char * h_escape_string(const struct _h_connection * conn, const char * unsafe);

/**
 * h_escape_string_with_quotes
 * Escapes a string and returns it ready to be inserted in the query
 * returned value must be h_h_free'd after use
 */
char * h_escape_string_with_quotes(const struct _h_connection * conn, const char * unsafe);
```

### Execute a SQL query

To execute a SQL query, you can use the function `h_execute_query` which will run the query in the database specified by the parameter `conn`. If a `result` parameter is specified, the result of the query (if any) will be stored in the `result` structure.

```c
/**
 * h_execute_query
 * Execute a query, set the result structure with the returned values if available
 * if result is NULL, the query is executed but no value will be returned
 * options available
 * H_OPTION_NONE (0): no option
 * H_OPTION_SELECT: Execute a prepare statement (sqlite only)
 * H_OPTION_EXEC: Execute an exec statement (sqlite only)
 * return H_OK on success
 */
int h_execute_query(const struct _h_connection * conn, const char * query, struct _h_result * result, int options);
```

### Result structure

The `struct _h_result` is a structure containing the values returned by a query. The definition of the structure is:

```c
/**
 * sql result structure
 */
struct _h_result {
  unsigned int nb_rows;
  unsigned int nb_columns;
  struct _h_data ** data;
};
```

The data value is a 2 dimensional array with `struct _h_data` variables. A `struct _h_data` is defined as:

```c
/**
 * sql data container
 */
struct _h_data {
  int type;
  void * t_data;
};
```

where `type` can be the following values:

```c
#define HOEL_COL_TYPE_INT    0
#define HOEL_COL_TYPE_DOUBLE 1
#define HOEL_COL_TYPE_TEXT   2
#define HOEL_COL_TYPE_DATE   3
#define HOEL_COL_TYPE_BLOB   4
#define HOEL_COL_TYPE_NULL   5
```

`t_data` will point to a `struct _h_type_*` corresponding to the type. The `struct _h_type_*` available are:

```c
/**
 * sql value integer type
 */
struct _h_type_int {
  int value;
};

/**
 * sql value double type
 */
struct _h_type_double {
  double value;
};

/**
 * sql value date/time type
 */
struct _h_type_datetime {
  struct tm value;
};

/**
 * sql value string type
 */
struct _h_type_text {
  char * value;
};

/**
 * sql value blob type
 */
struct _h_type_blob {
  size_t length;
  void * value;
};
```

### Clean results or data

To clean a result or a data structure, you can use its dedicated functions:

```c
/**
 * h_clean_result
 * Free all the memory allocated by the struct _h_result
 * return H_OK on success
 */
int h_clean_result(struct _h_result * result);

/**
 * h_clean_data
 * Free memory allocated by the struct _h_data
 * return H_OK on success
 */
int h_clean_data(struct _h_data * data);
```

### Get last id inserted

If you need the last id generated after an insert query, you can use the following function:

```c
/**
 * h_query_last_insert_id
 * return the id of the last inserted value
 * return a pointer to `struct _h_data *` on success, NULL otherwise.
 */
struct _h_data * h_query_last_insert_id(const struct _h_connection * conn);
```

### Additional query functions

You can use additional functions for specific needs. All these function will use `h_execute_query` but check input parameters before.

```c
/**
 * h_query_insert
 * Execute an insert query
 * return H_OK on success
 */
int h_query_insert(const struct _h_connection * conn, const char * query);

/**
 * h_query_update
 * Execute an update query
 * return H_OK on success
 */
int h_query_update(const struct _h_connection * conn, const char * query);

/**
 * h_query_delete
 * Execute an delete query
 * return H_OK on success
 */
int h_query_delete(const struct _h_connection * conn, const char * query);

/**
 * h_execute_query
 * Execute a select query, set the result structure with the returned values
 * return H_OK on success
 */
int h_query_select(const struct _h_connection * conn, const char * query, struct _h_result * result);
```

### Simple JSON queries

Hoel allows to use JSON objects for simple queries with `jansson` library. In the simple JSON queries, a JSON object called `json_t * j_query` is used to generate the query.

All `json_t *` returned and updated values must be free after use.

A `j_query` has the following form:
```javascript
{
  "table": "table_name"             // String, mandatory, the table name where the query is executed
  "columns": ["col1", "col2"]       // Array of strings, available for h_select, optional. If not specified,will be used
  "order_by": "col_name [asc|desc]" // String, available for h_select, specify the order by clause, optional
  "limit": integer_value            // Integer, available for h_select, specify the limit value, optional
  "offset"                          // Integer, available for h_select, specify the limit value, optional but available only if limit is set
  "values": [{                      // JSON object or JSON array of JSON objects, available for h_insert, mandatory, specify the values to update
    "col1": "value1",               // Generates col1='value1' for an update query
    "col2": value_integer,          // Generates col2=value_integer for an update query
    "col3", "value3",               // Generates col3='value3' for an update query
    "col4", null                    // Generates col4=NULL for an update query
  }]
  "set": {                          // JSON object, available for h_update, mandatory, specify the values to update
    "col1": "value1",               // Generates col1='value1' for an update query
    "col2": value_integer,          // Generates col2=value_integer for an update query
    "col3", "value3",               // Generates col3='value3' for an update query
    "col4", null                    // Generates col4=NULL for an update query
  }
  "where": {                        // JSON object, available for h_select, h_update and h_delete, mandatory, specify the where clause. All clauses are separated with an AND operator
    "col1": "value1",               // Generates col1='value1'
    "col2": value_integer,          // Generates col2=value_integer
    "col3": null,                   // Generates col3=NULL
    "col4": {                       // Generates col4<12
      "operator": "<",
      "value": 12
    },
    "col5": {                       // Generates col5 IS NOT NULL
      "operator": "NOT NULL"
    },
    "col6": {                       // Generates col6 LIKE '%value6%'
      "operator": "raw",
      "value": "LIKE '%value6%'"
    },
    "col7": {
      "operator": "IN",             // Generates col7 IN ('value1',42,4.2)
      "value": [                    // Values can be string, real or integer
        "value1",
        42,
        4.2
      ]
    }
  }
}
```

#### Where clause construction

A `where` clause is a JSON object containing a series of clauses. A clause can have 2 different forms:

- `col_name: value`
- `col_name: {operator: "operator_value", value: value}`

In the first case, `col_name: value`, the clause becomes `col_name = value`. Value is always escaped.

In the second case, `col_name: {operator: "operator_value", value: value}`, depending on the `operator` value, the clause can have different forms:
- `operator: "NOT NULL"`, the clause becomes `col_name IS NOT NULL`
- `operator: "raw"`, the `value` value becomes the clause itself, not escaped, for example in `{ "operator": "raw", "value": "LIKE '%value6%'" }`, the clause becomes `col6 LIKE '%value6%'`
- otherwise, the clause becomes `col_name operator value`, value is escaped

All clauses are separated by an `AND` operator.

As en example, here is a JSON object and its generated where clause:

JSON object:
```javascript
{
  "col1": "value1",
  "col2": 42,
  "col3": {
    "operator": ">=",
    "value": 55.5
  },
  "col4": {
    "operator": "raw",
    "value": "LIKE '%alu%'"
  }
}
```

SQL Where clause:
```sql
WHERE col1 = 'value1'
  AND col2 = 42
  AND col3 >= 55.5
  AND col4 LIKE '%alu%'
```

If you need less simple clauses, you can build it on your own and use the `h_execute_query` or the `h_execute_query_json` functions.

The simple JSON queries functions are:

```c
/**
 * h_select
 * Execute a select query
 * Uses a json_t * parameter for the query parameters
 * Store the result of the query in j_result if specified. j_result must be decref'd after use
 * Duplicate the generated query in generated_query if specified, must be h_free'd after use
 * return H_OK on success
 */
int h_select(const struct _h_connection * conn, const json_t * j_query, json_t ** j_result, char ** generated_query);

/**
 * h_insert
 * Execute an insert query
 * Uses a json_t * parameter for the query parameters
 * Duplicate the generated query in generated_query if specified, must be h_free'd after use
 * return H_OK on success
 */
int h_insert(const struct _h_connection * conn, const json_t * j_query, char ** generated_query);

/**
 * h_last_insert_id
 * return the id of the last inserted value
 * return a pointer to `json_t *` on success, NULL otherwise.
 * The returned value is of type JSON_INTEGER
 */
json_t * h_last_insert_id(const struct _h_connection * conn);

/**
 * h_update
 * Execute an update query
 * Uses a json_t * parameter for the query parameters
 * Duplicate the generated query in generated_query if specified, must be h_free'd after use
 * return H_OK on success
 */
int h_update(const struct _h_connection * conn, const json_t * j_query, char ** generated_query);

/**
 * h_delete
 * Execute a delete query
 * Uses a json_t * parameter for the query parameters
 * Duplicate the generated query in generated_query if specified, must be h_free'd after use
 * return H_OK on success
 */
int h_delete(const struct _h_connection * conn, const json_t * j_query, char ** generated_query);
```

#### JSON last insert id

The function `h_last_insert_id` returns the last inserted id in a `json_t *` format.

```c
/**
 * h_last_insert_id
 * return the id of the last inserted value
 * return a pointer to `json_t *` on success, NULL otherwise.
 * The returned value is of type JSON_INTEGER
 */
json_t * h_last_insert_id(const struct _h_connection * conn);
```

### Example source code

See `examples` folder for detailed sample source codes.
