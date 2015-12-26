/**
 * 
 * Hoel database abstraction library
 * 
 * hoel.h: structures and functions declarations
 * 
 * Copyright 2015 Nicolas Mora <mail@babelouest.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation;
 * version 2.1 of the License.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU GENERAL PUBLIC LICENSE for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#ifndef __HOEL_H__
#define __HOEL_H__

#define HOEL_VERSION 0.9

#include <string.h>
#include <jansson.h>

#define __USE_XOPEN
#include <time.h>

#include <yder.h>

#ifdef _HOEL_SQLITE
#define HOEL_DB_TYPE_SQLITE  0
#endif

#ifdef _HOEL_MARIADB
#define HOEL_DB_TYPE_MARIADB 1
#endif

#ifdef _HOEL_PGSQL
#define HOEL_DB_TYPE_PGSQL   2
#endif

#define HOEL_COL_TYPE_INT    0
#define HOEL_COL_TYPE_DOUBLE 1
#define HOEL_COL_TYPE_TEXT   2
#define HOEL_COL_TYPE_DATE   3
#define HOEL_COL_TYPE_BLOB   4
#define HOEL_COL_TYPE_NULL   5

#define H_OK                0  // No error
#define H_ERROR             1  // Generic error
#define H_ERROR_PARAMS      2  // Error in input parameters
#define H_ERROR_CONNECTION  3  // Error in database connection
#define H_ERROR_QUERY       4  // Error executing query
#define H_ERROR_MEMORY      99 // Error allocating memory

/**
 * handle container
 */
struct _h_connection {
  int type;
  void * connection;
};

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

/**
 * sql data container
 */
struct _h_data {
  int type;
  void * t_data;
};

/**
 * sql result structure
 */
struct _h_result {
  unsigned int nb_rows;
  unsigned int nb_columns;
  struct _h_data ** data;
};

/**
 * Close a database connection
 * return H_OK on success
 */
int h_close_db(struct _h_connection * conn);

/**
 * h_escape_string
 * Escapes a string
 * returned value must be free'd after use
 */
char * h_escape_string(const struct _h_connection * conn, const char * unsafe);

/**
 * h_execute_query
 * Execute a query, set the result structure with the returned values
 * if result is NULL, the query is executed but no value will be returned
 * return H_OK on success
 */
int h_execute_query(const struct _h_connection * conn, const char * query, struct _h_result * result);

/**
 * h_query_insert
 * Execute an insert query
 * return H_OK on success
 */
int h_query_insert(const struct _h_connection * conn, const char * query);

/**
 * h_last_insert_id
 * return the id of the last inserted value
 * return a pointer to `struct _h_data *` on success, NULL otherwise.
 */
struct _h_data * h_last_insert_id(const struct _h_connection * conn);

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
 * h_query_select
 * Execute a select query, set the result structure with the returned values
 * return H_OK on success
 */
int h_query_select(const struct _h_connection * conn, const char * query, struct _h_result * result);

/**
 * h_execute_query_json
 * Execute a query, set the returned values in the json result
 * return H_OK on success
 */
int h_execute_query_json(const struct _h_connection * conn, const char * query, json_t ** j_result);

/**
 * h_query_select_json
 * Execute a select query, set the returned values in the json results
 * return H_OK on success
 */
int h_query_select_json(const struct _h_connection * conn, const char * query, json_t ** j_result);

/**
 * h_select
 * Execute a select using a table name for the FROM keyword, a json array for the columns, and a json object for the WHERE keyword
 * where must be a where_type json object
 * return H_OK on success
 */
int h_select(const struct _h_connection * conn, const char * table, json_t * cols, json_t * where, json_t ** j_result, char ** generated_query);

/**
 * h_insert
 * Insert data using a json object and a table name
 * data must be an object or an array of objects
 * return H_OK on success
 */
int h_insert(const struct _h_connection * conn, const char * table, json_t * data, char ** generated_query);

/**
 * h_update
 * Update data using a json object and a table name and a where clause
 * data must be an object, where must be a where_type json object
 * return H_OK on success
 */
int h_update(const struct _h_connection * conn, const char * table, json_t * set, json_t * where, char ** generated_query);

/**
 * h_delete
 * Delete data using a table name and a where clause
 * where must be a where_type json object
 * return H_OK on success
 */
int h_delete(const struct _h_connection * conn, const char * table, json_t * where, char ** generated_query);

/**
 * Add a new struct _h_data * to an array of struct _h_data *, which already has cols columns
 * return H_OK on success
 */
int h_row_add_data(struct _h_data ** result, struct _h_data * data, int cols);

/**
 * Add a new row of struct _h_data * in a struct _h_result *
 * return H_OK on success
 */
int h_result_add_row(struct _h_result * result, struct _h_data * row, int rows);

/**
 * Allocate memory for a new struct _h_data * containing an int
 * return pointer to the new structure
 * return NULL on error
 */
struct _h_data * h_new_data_int(const int value);

/**
 * Allocate memory for a new struct _h_data * containing a double
 * return pointer to the new structure
 * return NULL on error
 */
struct _h_data * h_new_data_double(const double value);

/**
 * Allocate memory for a new struct _h_data * containing a text
 * return pointer to the new structure
 * return NULL on error
 */
struct _h_data * h_new_data_text(const char * value);

/**
 * Allocate memory for a new struct _h_data * containing a blob
 * return pointer to the new structure
 * return NULL on error
 */
struct _h_data * h_new_data_blob(const void * value, const size_t length);

/**
 * Allocate memory for a new struct _h_data * containing a date time structure
 * return pointer to the new structure
 * return NULL on error
 */
struct _h_data * h_new_data_datetime(const struct tm * datetime);

/**
 * Allocate memory for a new struct _h_data * containing a null value
 * return pointer to the new structure
 * return NULL on error
 */
struct _h_data * h_new_data_null();

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

/**
 * h_clean_data_full
 * Free memory allocated by the struct _h_data and the struct _h_data pointer
 * return H_OK on success
 */
int h_clean_data_full(struct _h_data * data);

/**
 * h_clean_connection
 * free memory allocated by the struct _h_connection
 * return H_OK on success
 */
int h_clean_connection(struct _h_connection * conn);

/**
 * trim_whitespace_and_double_quotes
 * Return the string without its beginning and ending whitespaces or double quotes
 */
char * trim_whitespace_and_double_quotes(char *str);

/**
 * Implementation of sprintf that return a malloc'd char *  with the string construction
 * because life is too short to use 3 lines instead of 1
 * but don't forget to free the returned value after use!
 */
char * h_msprintf(const char * message, ...);

#ifdef _HOEL_SQLITE
/**
 * h_connect_sqlite
 * Opens a database connection to a sqlite3 db file
 * return pointer to a struct _h_connection * on sucess, NULL on error
 */
struct _h_connection * h_connect_sqlite(const char * db_path);

/**
 * close a sqlite3 connection
 */
void h_close_sqlite(struct _h_connection * conn);

/**
 * escape a string
 * returned value must be free'd after use
 */
char * h_escape_string_sqlite(const struct _h_connection * conn, const char * unsafe);

/**
 * Return the id of the last inserted value
 */
int h_last_insert_id_sqlite(const struct _h_connection * conn);

/**
 * h_execute_query_sqlite
 * Execute a query on a sqlite connection, set the result structure with the returned values
 * Should not be executed by the user because all parameters are supposed to be correct
 * if result is NULL, the query is executed but no value will be returned
 * return H_OK on success
 */
int h_execute_query_sqlite(const struct _h_connection * conn, const char * query, struct _h_result * result);

/**
 * h_execute_query_json_sqlite
 * Execute a query on a sqlite connection, set the returned values in the json result
 * Should not be executed by the user because all parameters are supposed to be correct
 * return H_OK on success
 */
int h_execute_query_json_sqlite(const struct _h_connection * conn, const char * query, json_t ** j_result);
#endif

#ifdef _HOEL_MARIADB
/**
 * h_connect_mariadb
 * Opens a database connection to a mariadb server
 * return pointer to a struct _h_connection * on sucess, NULL on error
 */
struct _h_connection * h_connect_mariadb(char * host, char * user, char * passwd, char * db, unsigned int port, char * unix_socket);

/**
 * close connection to database
 */
void h_close_mariadb(struct _h_connection * conn);

/**
 * escape a string
 * returned value must be free'd after use
 */
char * h_escape_string_mariadb(const struct _h_connection * conn, const char * unsafe);

/**
 * Return the id of the last inserted value
 */
int h_last_insert_id_mariadb(const struct _h_connection * conn);
/**
 * h_execute_query_mariadb
 * Execute a query on a mariadb connection, set the result structure with the returned values
 * Should not be executed by the user because all parameters are supposed to be correct
 * if result is NULL, the query is executed but no value will be returned
 * return H_OK on success
 */
int h_execute_query_mariadb(const struct _h_connection * conn, const char * query, struct _h_result * result);

/**
 * h_execute_query_json_mariadb
 * Execute a query on a mariadb connection, set the returned values in the json result
 * Should not be executed by the user because all parameters are supposed to be correct
 * return H_OK on success
 */
int h_execute_query_json_mariadb(const struct _h_connection * conn, const char * query, json_t ** j_result);

/**
 * h_get_mariadb_value
 * convert value into a struct _h_data * depening on the m_type given
 * returned value must be free'd with h_clean_data_full after use
 */
struct _h_data * h_get_mariadb_value(const char * value, const unsigned long length, const int m_type);
#endif

#ifdef _HOEL_PGSQL
/**
 * h_connect_pgsql
 * Opens a database connection to a PostgreSQL server
 * return pointer to a struct _h_connection * on sucess, NULL on error
 */
struct _h_connection * h_connect_pgsql(char * conninfo);

/**
 * h_execute_query_pgsql
 * Execute a query on a pgsql connection, set the result structure with the returned values
 * Should not be executed by the user because all parameters are supposed to be correct
 * if result is NULL, the query is executed but no value will be returned
 * return H_OK on success
 */
int h_execute_query_pgsql(const struct _h_connection * conn, const char * query, struct _h_result * result);

/**
 * h_execute_query_json_pgsql
 * Execute a query on a pgsql connection, set the returned values in the json results
 * Should not be executed by the user because all parameters are supposed to be correct
 * return H_OK on success
 */
int h_execute_query_json_pgsql(const struct _h_connection * conn, const char * query, json_t ** j_result);

/**
 * close a pgsql connection
 */
void h_close_pgsql(struct _h_connection * conn);

/**
 * escape a string
 * returned value must be free'd after use
 */
char * h_escape_string_pgsql(const struct _h_connection * conn, const char * unsafe);
#endif

#endif // __HOEL_H__
