# Hoel

Database abstraction library written in C.

Simple and easy to use database access library. Works with SQLite 3, MariaDB/Mysql and (barely) PostgreSQL databases.

# Installation

Clone, compile and install yder library.

```shell
$ git clone https://github.com/babelouest/yder.git
$ cd yder
$ make
$ sudo make install
```

Install [Jansson](http://www.digip.org/jansson/) library for json manipulation. On a debian-based platform, run the following command:

```shell
$ sudo apt-get install libjansson-dev
```

Download hoel from github repository.

```shell
$ git clone https://github.com/babelouest/hoel.git
```

Compile hoel for the backend you need, go to hoel source folder, depending on your database backend needs, follow these instructions.

## SQLite 3

Install libsqlite3-dev and uncomment the following lines in the `src/Makefile`

```Makefile
# HAS_SQLITE=-D_HOEL_SQLITE
# LIBS_SQLITE=-lsqlite3
```

## MariaDB/Mysql

Install libmysqlclient-dev and uncomment the following lines in the `src/Makefile`

```Makefile
# FLAGS_MARIADB=-D_HOEL_MARIADB -I/usr/include/mysql/
# LIBS_MYSQL=-lmysqlclient
```

## Postgre SQL

Install libpq-dev and uncomment the following lines in the `src/Makefile`

```Makefile
# HAS_PGSQL=-D_HOEL_PGSQL -I/usr/include/postgresql/
# LIBS_PGSQL=-lpq
```

### Postgre SQL limitations

For some reasons, the Postgre SQL backend has some limitations. The `h_last_insert_id` doesn't work, and a select statement returns only string values. The reason is I couldn't find on the documentation how to implement those...

### Use different backends

You can use different backends at the same time, simply install the required libraries and uncomment all the required backend requirements in the `src/Makefile`.

## Compile and install

```
$ cd src
$ make
$ sudo make install
```

By default, the shared library and the header file will be installed in the `/usr/local` location. To change this setting, you can modify the `PREFIX` value in the `src/Makefile`.

# API Documentation

## Header files and compilation

To use hoel in your code, you must use the `#define` corresponding to your backend before including the file `hoel.h`. For example:

```c
#define _HOEL_SQLITE
#include <hoel.h>
```

If you want to use different backends in your source code, just add its `#define` before including `hoel.h`.

```c
#define _HOEL_SQLITE
#define _HOEL_MARIADB
#define _HOEL_PGSQL
#include <hoel.h>
```

Use the flag `-lhoel` to include hoel library in the link edition of your compilation.

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

### Initialisation

To generate a connection to a database, use its dedicated function

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
 * return pointer to a struct _h_connection * on sucess, NULL on error
 */
struct _h_connection * h_connect_mariadb(char * host, char * user, char * passwd, char * db, unsigned int port, char * unix_socket);

/**
 * h_connect_pgsql
 * Opens a database connection to a PostgreSQL server
 * return pointer to a struct _h_connection * on sucess, NULL on error
 */
struct _h_connection * h_connect_pgsql(char * conninfo);
```

All these functions return a struct _h_connection * on success. This pointer will be needed on every call to hoel functions.

When you no longer need your connection, close it using the function `h_close_db`. This will close the connection to the database and free the memory allocated by the connection.

```c
/**
 * Close a database connection
 * return H_OK on success
 */
int h_close_db(struct _h_connection * conn);
```

### Execute a SQL query

To execute a SQL query, you can use the function `h_execute_query` which will run the query in the database specified by the parameter `conn`. If a `result` parameter is specified, the result of the query (if any) will be stored in the `result` structure.

```c
/**
 * h_execute_query
 * Execute a query, set the result structure with the returned values
 * if result is NULL, the query is executed but no value will be returned
 * return H_OK on success
 */
int h_execute_query(const struct _h_connection * conn, const char * query, struct _h_result * result);
```

### Escape string

If you need to escape parameters, you can use the function `h_escape_string` the returned value must be free'd after use

```c
/**
 * h_escape_string
 * Escapes a string
 * returned value must be free'd after use
 */
char * h_escape_string(const struct _h_connection * conn, const char * unsafe);
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
 * h_last_insert_id
 * return the id of the last inserted value
 * return a pointer to `struct _h_data *` on success, NULL otherwise.
 */
struct _h_data * h_last_insert_id(const struct _h_connection * conn);
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

### Simple json queries

Hoel allows to use json objects for simple queries with `jansson` library. In the simple json queries, json objects are used for `where` clauses, `set` clauses, `select` columns and `results`.

#### Where clause construction

A `where` clause is a json object containing a series of clauses. A clause can have 2 different forms:

- `col_name: value`
- `col_name: {operator: "operator_value", value: value}`

In the first case, `col_name: value`, the clause becomes `col_name = value`.

In the second case, `col_name: {operator: "operator_value", value: value}`, the clause becomes `col_name operator_value value`.

All clauses are separated by an `AND` operator. All clauses values are automatically escaped.

As en axample, here is a json object and its generated where clause:

JSON object:
```json
{
  col1: "value1",
  col2: 42,
  col3: {
    operator: ">=",
    value: 55.5
  },
  col4: {
    operator: "LIKE",
    value: "%alu%"
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

The simple json queries functions are:

```c
/**
 * h_execute_query_json
 * Execute a query, set the returned values in the json result
 * return H_OK on success
 */
int h_execute_query_json(const struct _h_connection * conn, const char * query, json_t ** j_result);

/**
 * h_select
 * Execute a select using a table name for the FROM keyword, a json array for the columns, and a json object for the WHERE keyword
 * where must be a where_type json object
 * return H_OK on success
 */
int h_select(const struct _h_connection * conn, const char * table, json_t * cols, json_t * where, json_t ** j_result);

/**
 * h_insert
 * Insert data using a json object and a table name
 * data must be an object or an array of objects
 * return H_OK on success
 */
int h_insert(const struct _h_connection * conn, const char * table, json_t * data);

/**
 * h_update
 * Update data using a json object and a table name and a where clause
 * data must be an object, where must be a where_type json object
 * return H_OK on success
 */
int h_update(const struct _h_connection * conn, const char * table, json_t * set, json_t * where);

/**
 * h_delete
 * Delete data using a table name and a where clause
 * where must be a where_type json object
 * return H_OK on success
 */
int h_delete(const struct _h_connection * conn, const char * table, json_t * where);
```

### Example source code

See `examples` folder for detailed sample source codes.
