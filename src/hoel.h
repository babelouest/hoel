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

#define HOEL_VERSION 0.5

#include <string.h>

#define __USE_XOPEN
#include <time.h>

#define HOEL_DB_TYPE_SQLITE  0
#define HOEL_DB_TYPE_MARIADB 1
#define HOEL_DB_TYPE_PGSQL   2

#define HOEL_COL_TYPE_INT    0
#define HOEL_COL_TYPE_DOUBLE 1
#define HOEL_COL_TYPE_TEXT   2
#define HOEL_COL_TYPE_DATE   3
#define HOEL_COL_TYPE_BLOB   4
#define HOEL_COL_TYPE_NULL   5

#define H_OK                0
#define H_ERROR             1
#define H_ERROR_PARAMS      2
#define H_ERROR_CONNECTION  3
#define H_ERROR_DISABLED    4
#define H_ERROR_QUERY       5
#define H_ERROR_MEMORY      99

/**
 * handle container
 */
struct _h_connection {
  int type;
  int enabled;
  void * connection;
};

/**
 * query structure
 */
struct _h_query {
  int type;
  unsigned int length;
  char * query;
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
  int result_query;
  struct _h_data ** data;
};

/**
 * h_connect_sqlite
 * Opens a database connection to a sqlite3 db file
 * return H_OK on success
 */
struct _h_connection * h_connect_sqlite(const char * db_path);

/**
 * h_connect_mariadb
 * Opens a database connection to a mariadb server
 * return H_OK on success
 */
struct _h_connection * h_connect_mariadb(char * host, char * user, char * passwd, char * db, unsigned int port, char * unix_socket);

/**
 * h_connect_pgsql
 * Opens a database connection to a PostgreSQL server
 * return H_OK on success
 */
struct _h_connection * h_connect_pgsql(char * conninfo);

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
 * h_execute_query_sqlite
 * Execute a query on a sqlite connection, set the result structure with the returned values
 * Should not be executed by the user because all parameters are supposed to be correct
 * if result is NULL, the query is executed but no value will be returned
 * return H_OK on success
 */
int h_execute_query_sqlite(const struct _h_connection * conn, const char * query, struct _h_result * result);

/**
 * h_execute_query_mariadb
 * Execute a query on a mariadb connection, set the result structure with the returned values
 * Should not be executed by the user because all parameters are supposed to be correct
 * if result is NULL, the query is executed but no value will be returned
 * return H_OK on success
 */
int h_execute_query_mariadb(const struct _h_connection * conn, const char * query, struct _h_result * result);

/**
 * h_execute_query_pgsql
 * Execute a query on a pgsql connection, set the result structure with the returned values
 * Should not be executed by the user because all parameters are supposed to be correct
 * if result is NULL, the query is executed but no value will be returned
 * return H_OK on success
 */
int h_execute_query_pgsql(const struct _h_connection * conn, const char * query, struct _h_result * result);

/**
 * h_query_insert
 * Execute an insert query
 * return H_OK on success
 */
int h_query_insert(const struct _h_connection * conn, const char * query);

/**
 * h_last_insert_id
 * return the id of the last inserted value
 * return H_OK on success
 */
struct _h_data * h_last_insert_id(const struct _h_connection * conn, struct _h_result * result);

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
 * h_get_mariadb_value
 * convert value into a struct _h_data * depening on the m_type given
 * returned value must be free'd with h_clean_data_full after use
 */
struct _h_data * h_get_mariadb_value(const char * value, const unsigned long length, const int m_type);

#endif // __HOEL_H__
