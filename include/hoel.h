/**
 * 
 * Hoel database abstraction library
 * 
 * hoel.h: structures and functions declarations
 * 
 * Copyright 2015-2018 Nicolas Mora <mail@babelouest.org>
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

#define HOEL_VERSION 1.4.3

#include <jansson.h>

#ifndef __USE_XOPEN
  #define __USE_XOPEN
#endif
#include <time.h>
#include <pthread.h>

/** Angharad libraries **/
#include <yder.h>
#include <orcania.h>

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
#define HOEL_COL_TYPE_BOOL   5
#define HOEL_COL_TYPE_NULL   5

#define H_OK                0  /* No error */
#define H_ERROR             1  /* Generic error */
#define H_ERROR_PARAMS      2  /* Error in input parameters */
#define H_ERROR_CONNECTION  3  /* Error in database connection */
#define H_ERROR_QUERY       4  /* Error executing query */
#define H_ERROR_MEMORY      99 /* Error allocating memory */

#define H_OPTION_NONE   0x0000 /* Nothing whatsoever */
#define H_OPTION_SELECT 0x0001 /* Execute a SELECT statement */
#define H_OPTION_EXEC   0x0010 /* Execute an INSERT, UPDATE or DELETE statement */

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
 * free data allocated by hoel functions
 */
void h_free(void * data);

/**
 * h_escape_string
 * Escapes a string
 * returned value must be free'd after use
 */
char * h_escape_string(const struct _h_connection * conn, const char * unsafe);

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

/**
 * h_query_insert
 * Execute an insert query
 * return H_OK on success
 */
int h_query_insert(const struct _h_connection * conn, const char * query);

/**
 * h_query_last_insert_id
 * return the id of the last inserted value
 * return a pointer to `struct _h_data *` on success, NULL otherwise.
 */
struct _h_data * h_query_last_insert_id(const struct _h_connection * conn);

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
 * json queries
 * The following functions run a sql query based on a json_t * object for input parameters
 * The input parameter is called j_query
 * If the j_query is well-formed, the query is executed and if available and specified, the result is stored into the j_result object. j_result must be decref'd after use
 * Also, the sql query generated is stored into generated_query if specified, generated_query must be free'd after use
 * The query execution result is returned by the function
 * 
 * A j_query has the following form
 * {
 *   "table": "table_name"             // String, mandatory, the table name where the query is executed
 *   "columns": ["col1", "col2"]       // Array of strings, available for h_select, optional. If not specified,will be used
 *   "order_by": "col_name [asc|desc]" // String, available for h_select, specify the order by clause, optional
 *   "limit": integer_value            // Integer, available for h_select, specify the limit value, optional
 *   "offset"                          // Integer, available for h_select, specify the limit value, optional but available only if limit is set
 *   "values": [{                      // json object or json array of json objects, available for h_insert, mandatory, specify the values to update
 *     "col1": "value1",               // Generates col1='value1' for an update query
 *     "col2": value_integer,          // Generates col2=value_integer for an update query
 *     "col3", "value3",               // Generates col3='value3' for an update query
 *     "col4", null                    // Generates col4=NULL for an update query
 *   }]
 *   "set": {                          // json object, available for h_update, mandatory, specify the values to update
 *     "col1": "value1",               // Generates col1='value1' for an update query
 *     "col2": value_integer,          // Generates col2=value_integer for an update query
 *     "col3", "value3",               // Generates col3='value3' for an update query
 *     "col4", null                    // Generates col4=NULL for an update query
 *   }
 *   "where": {                        // json object, available for h_select, h_update and h_delete, mandatory, specify the where clause. All clauses are separated with an AND operator
 *     "col1": "value1",               // Generates col1='value1'
 *     "col2": value_integer,          // Generates col2=value_integer
 *     "col3": null,                   // Generates col3=NULL
 *     "col4", {                       // Generates col4<12
 *       "operator": "<",
 *       "value": 12
 *     },
 *     "col5", {                       // Generates col5 IS NOT NULL
 *       "operator": "NOT NULL"
 *     },
 *     "col6", {                       // Generates col6 LIKE '%value6%'
 *       "operator": "raw",
 *       "value": "LIKE '%value6%'"
 *     }
 *   }
 * }
 */

/**
 * h_select
 * Execute a select query
 * Uses a json_t * parameter for the query parameters
 * Store the result of the query in j_result if specified. j_result must be decref'd after use
 * Duplicate the generated query in generated_query if specified, must be free'd after use
 * return H_OK on success
 */
int h_select(const struct _h_connection * conn, const json_t * j_query, json_t ** j_result, char ** generated_query);

/**
 * h_insert
 * Execute an insert query
 * Uses a json_t * parameter for the query parameters
 * Duplicate the generated query in generated_query if specified, must be free'd after use
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
 * Duplicate the generated query in generated_query if specified, must be free'd after use
 * return H_OK on success
 */
int h_update(const struct _h_connection * conn, const json_t * j_query, char ** generated_query);

/**
 * h_delete
 * Execute a delete query
 * Uses a json_t * parameter for the query parameters
 * Duplicate the generated query in generated_query if specified, must be free'd after use
 * return H_OK on success
 */
int h_delete(const struct _h_connection * conn, const json_t * j_query, char ** generated_query);

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
 * h_select_query_sqlite
 * Execute a select query on a sqlite connection, set the result structure with the returned values
 * Should not be executed by the user because all parameters are supposed to be correct
 * if result is NULL, the query is executed but no value will be returned
 * Useful for SELECT statements
 * return H_OK on success
 */
int h_select_query_sqlite(const struct _h_connection * conn, const char * query, struct _h_result * result);

/**
 * h_exec_query_sqlite
 * Execute a query on a sqlite connection
 * Should not be executed by the user because all parameters are supposed to be correct
 * No result is returned, useful for single INSERT, UPDATE or DELETE statements
 * return H_OK on success
 */
int h_exec_query_sqlite(const struct _h_connection * conn, const char * query);

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
 * return pointer to a struct _h_connection * on success, NULL on error
 */
struct _h_connection * h_connect_mariadb(const char * host, const char * user, const char * passwd, const char * db, const unsigned int port, const char * unix_socket);

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

/**
 * Return the id of the last inserted value
 * Assuming you use sequences for automatically generated ids
 */
int h_last_insert_id_pgsql(const struct _h_connection * conn);
#endif

#endif /* __HOEL_H__ */
