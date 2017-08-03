/**
 * 
 * Hoel database abstraction library
 * 
 * hoel-pgsql.c: Postgre SQL specific functions
 * 
 * Copyright 2015-2016 Nicolas Mora <mail@babelouest.org>
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
#include "hoel.h"
#include "h-private.h"

#ifdef _HOEL_PGSQL
// PostgreSQL library includes
#include <libpq-fe.h>
#include <string.h>

/**
 * Postgre SQL handle
 */
struct _h_pgsql {
  char * conninfo;
  PGconn * db_handle;
};

/**
 * h_connect_pgsql
 * Opens a database connection to a PostgreSQL server
 * return pointer to a struct _h_connection * on sucess, NULL on error
 */
struct _h_connection * h_connect_pgsql(char * conninfo) {
  // TODO get oids and types
  struct _h_connection * conn = NULL;
  if (conninfo != NULL) {
    conn = malloc(sizeof(struct _h_connection));
    if (conn == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Hoel - Error allocating memory for conn");
      return NULL;
    }
    
    conn->type = HOEL_DB_TYPE_PGSQL;
    conn->connection = malloc(sizeof(struct _h_pgsql));
    if (conn->connection == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Hoel - Error allocating memory for conn->connection");
      free(conn);
      return NULL;
    }
    ((struct _h_pgsql *)conn->connection)->db_handle = PQconnectdb(conninfo);
    
    if (PQstatus(((struct _h_pgsql *)conn->connection)->db_handle) != CONNECTION_OK) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Hoel - Error connecting to PostgreSQL Database");
      y_log_message(Y_LOG_LEVEL_DEBUG, "Hoel - Error message: \"%s\"", PQerrorMessage(((struct _h_pgsql *)conn->connection)->db_handle));
      PQfinish(((struct _h_pgsql *)conn->connection)->db_handle);
      free(conn->connection);
      free(conn);
      return NULL;
    }
  }
  return conn;
}

/**
 * close a pgsql connection
 */
void h_close_pgsql(struct _h_connection * conn) {
  PQfinish(((struct _h_pgsql *)conn->connection)->db_handle);
}

/**
 * escape a string
 * returned value must be free'd after use
 */
char * h_escape_string_pgsql(const struct _h_connection * conn, const char * unsafe) {
  return PQescapeLiteral(((struct _h_pgsql *)conn->connection)->db_handle, unsafe, strlen(unsafe));
}

/**
 * h_execute_query_pgsql
 * Execute a query on a pgsql connection, set the result structure with the returned values
 * Should not be executed by the user because all parameters are supposed to be correct
 * if result is NULL, the query is executed but no value will be returned
 * return H_OK on success
 */
int h_execute_query_pgsql(const struct _h_connection * conn, const char * query, struct _h_result * result) {
  PGresult *res;
  int nfields, ntuples, i, j, h_res;
  struct _h_data * data, * cur_row = NULL;
  
  res = PQexec(((struct _h_pgsql *)conn->connection)->db_handle, query);
  if (PQresultStatus(res) != PGRES_TUPLES_OK) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error executing sql query");
    y_log_message(Y_LOG_LEVEL_DEBUG, "Error message: \"%s\"", PQerrorMessage(((struct _h_pgsql *)conn->connection)->db_handle));
    y_log_message(Y_LOG_LEVEL_DEBUG, "Query: \"%s\"", query);
    return H_ERROR_QUERY;
  }
  nfields = PQnfields(res);
  ntuples = PQntuples(res);

  if (result != NULL) {
    result->nb_rows = 0;
    result->nb_columns = nfields;
    result->data = NULL;
    for(i = 0; i < ntuples; i++) {
      cur_row = NULL;
      for(j = 0; j < nfields; j++) {
        char * val = PQgetvalue(res, i, j);
        if (val == NULL || strlen(val) == 0) {
          data = h_new_data_null();
        } else {
          data = h_new_data_text(PQgetvalue(res, i, j));
        }
        h_res = h_row_add_data(&cur_row, data, j);
        h_clean_data_full(data);
        if (h_res != H_OK) {
          PQclear(res);
          return h_res;
        }
      }
      h_res = h_result_add_row(result, cur_row, i);
      if (h_res != H_OK) {
        PQclear(res);
        return h_res;
      }
    }
  }
  PQclear(res);
  return H_OK;
}

/**
 * h_execute_query_json_pgsql
 * Execute a query on a pgsql connection, set the returned values in the json results
 * Should not be executed by the user because all parameters are supposed to be correct
 * return H_OK on success
 */
int h_execute_query_json_pgsql(const struct _h_connection * conn, const char * query, json_t ** j_result) {
  PGresult *res;
  int nfields, ntuples, i, j;
  json_t * j_data;
  
  if (j_result == NULL) {
    return H_ERROR_PARAMS;
  }
  
  *j_result = json_array();
  if (*j_result == NULL) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Hoel - Error allocating memory for *j_result");
    return H_ERROR_MEMORY;
  }
  
  res = PQexec(((struct _h_pgsql *)conn->connection)->db_handle, query);
  if (PQresultStatus(res) != PGRES_TUPLES_OK) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error executing sql query");
    y_log_message(Y_LOG_LEVEL_DEBUG, "Error message: \"%s\"", PQerrorMessage(((struct _h_pgsql *)conn->connection)->db_handle));
    y_log_message(Y_LOG_LEVEL_DEBUG, "Query: \"%s\"", query);
    return H_ERROR_QUERY;
  }
  nfields = PQnfields(res);
  ntuples = PQntuples(res);

  for(i = 0; i < ntuples; i++) {
    j_data = json_object();
    if (j_data == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Hoel - Error allocating memory for j_data");
      PQclear(res);
      return H_ERROR_MEMORY;
    }
    for(j = 0; j < nfields; j++) {
      char * val = PQgetvalue(res, i, j);
      if (val == NULL || strlen(val) == 0) {
        json_object_set_new(j_data, PQfname(res, j), json_null());
      } else {
        json_object_set_new(j_data, PQfname(res, j), json_string(PQgetvalue(res, i, j)));
      }
    }
    json_array_append_new(*j_result, j_data);
    j_data = NULL;
  }
  PQclear(res);
  return H_OK;
}
#endif
