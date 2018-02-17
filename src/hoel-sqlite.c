/**
 * 
 * Hoel database abstraction library
 * 
 * hoel-sqlite.c: Sqlite3 specific functions
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
#include "../include/hoel.h"
#include "../include/h-private.h"

#ifdef _HOEL_SQLITE

#include <sqlite3.h>
#include <string.h>

/**
 * SQLite handle
 */
struct _h_sqlite {
  sqlite3 * db_handle;
};

/**
 * h_connect_sqlite
 * Opens a database connection to a sqlite3 db file
 * return pointer to a struct _h_connection * on sucess, NULL on error
 */
struct _h_connection * h_connect_sqlite(const char * db_path) {
  struct _h_connection * conn = NULL;
  if (db_path != NULL) {
    conn = malloc(sizeof(struct _h_connection));
    if (conn == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "h_connect_sqlite - Error allocating resources");
      return NULL;
    }
    
    conn->type = HOEL_DB_TYPE_SQLITE;
    conn->connection = malloc(sizeof(struct _h_sqlite));
    if (conn->connection == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "h_connect_sqlite - Error allocating resources");
      free(conn);
      return NULL;
    }
    if (sqlite3_open_v2(db_path, &((struct _h_sqlite *)conn->connection)->db_handle, SQLITE_OPEN_READWRITE, NULL) != SQLITE_OK) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error connecting to sqlite3 database, path: %s", db_path);
      y_log_message(Y_LOG_LEVEL_DEBUG, "Error code: %d, message: \"%s\"", 
                             sqlite3_errcode(((struct _h_sqlite *)conn->connection)->db_handle), 
                             sqlite3_errmsg(((struct _h_sqlite *)conn->connection)->db_handle));
      sqlite3_close(((struct _h_sqlite *)conn->connection)->db_handle);
      free(conn);
      return NULL;
    } else {
      return conn;
    }
  }
  return conn;
}

/**
 * close a sqlite3 connection
 */
void h_close_sqlite(struct _h_connection * conn) {
  sqlite3_close(((struct _h_sqlite *)conn->connection)->db_handle);
}

/**
 * escape a string
 * returned value must be free'd after use
 */
char * h_escape_string_sqlite(const struct _h_connection * conn, const char * unsafe) {
  char * tmp = sqlite3_mprintf("%q", unsafe), * ret;
  if (tmp == NULL) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error escaping string: %s", unsafe);
    y_log_message(Y_LOG_LEVEL_DEBUG, "Error code: %d, message: \"%s\"", 
                           sqlite3_errcode(((struct _h_sqlite *)conn->connection)->db_handle), 
                           sqlite3_errmsg(((struct _h_sqlite *)conn->connection)->db_handle));
    return NULL;
  }
  ret = o_strdup(tmp);
  sqlite3_free(tmp);
  if (ret == NULL) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error escaping (o_strdup)");
  }
  return ret;
}

/**
 * Return the id of the last inserted value
 */
int h_last_insert_id_sqlite(const struct _h_connection * conn) {
  return sqlite3_last_insert_rowid(((struct _h_sqlite *)conn->connection)->db_handle);
}

/**
 * h_select_query_sqlite
 * Execute a select query on a sqlite connection, set the result structure with the returned values
 * Should not be executed by the user because all parameters are supposed to be correct
 * if result is NULL, the query is executed but no value will be returned
 * Useful for SELECT statements
 * return H_OK on success
 */
int h_select_query_sqlite(const struct _h_connection * conn, const char * query, struct _h_result * result) {
  sqlite3_stmt *stmt;
  int sql_result, row_result, nb_columns, col, row, res;
  struct _h_data * data = NULL, * cur_row = NULL;
  
  sql_result = sqlite3_prepare_v2(((struct _h_sqlite *)conn->connection)->db_handle, query, strlen(query)+1, &stmt, NULL);
  
  if (sql_result == SQLITE_OK) {
    nb_columns = sqlite3_column_count(stmt);
    row = 0;
    if (result != NULL) {
      row_result = sqlite3_step(stmt);
      /* Filling result object with results in array format */
      result->nb_rows = 0;
      result->nb_columns = nb_columns;
      result->data = NULL;
      while (row_result == SQLITE_ROW) {
        cur_row = NULL;
        for (col = 0; col < nb_columns; col++) {
          data = NULL;
          switch (sqlite3_column_type(stmt, col)) {
            case SQLITE_INTEGER:
              data = h_new_data_int(sqlite3_column_int(stmt, col));
              break;
            case SQLITE_FLOAT:
              data = h_new_data_double(sqlite3_column_double(stmt, col));
              break;
            case SQLITE_BLOB:
              data = h_new_data_blob(sqlite3_column_blob(stmt, col), sqlite3_column_bytes(stmt, col));
              break;
            case SQLITE3_TEXT:
              data = h_new_data_text((char*)sqlite3_column_text(stmt, col));
              break;
            case SQLITE_NULL:
              data = h_new_data_null();
            default:
              break;
          }
          if (data == NULL) {
            sqlite3_finalize(stmt);
            h_clean_data_full(data);
            return H_ERROR_MEMORY;
          }
          res = h_row_add_data(&cur_row, data, col);
          h_clean_data_full(data);
          if (res != H_OK) {
            sqlite3_finalize(stmt);
            return res;
          }
        }
        res = h_result_add_row(result, cur_row, row);
        cur_row = NULL;
        if (res != H_OK) {
          sqlite3_finalize(stmt);
          return res;
        }
        row_result = sqlite3_step(stmt);
        row++;
      }
    }
    sqlite3_finalize(stmt);
    return H_OK;
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error executing sql query");
    y_log_message(Y_LOG_LEVEL_DEBUG, "Error code: %d, message: \"%s\"", 
                                   sqlite3_errcode(((struct _h_sqlite *)conn->connection)->db_handle), 
                                   sqlite3_errmsg(((struct _h_sqlite *)conn->connection)->db_handle));
    y_log_message(Y_LOG_LEVEL_DEBUG, "Query: \"%s\"", query);
    sqlite3_finalize(stmt);
    return H_ERROR_QUERY;
  }
}

/**
 * h_exec_query_sqlite
 * Execute a query on a sqlite connection
 * Should not be executed by the user because all parameters are supposed to be correct
 * No result is returned, useful for single INSERT, UPDATE or DELETE statements
 * return H_OK on success
 */
int h_exec_query_sqlite(const struct _h_connection * conn, const char * query) {
  return ((sqlite3_exec(((struct _h_sqlite *)conn->connection)->db_handle, query, NULL, NULL, NULL) == SQLITE_OK)?H_OK:H_ERROR_QUERY);
  
}

/**
 * h_execute_query_json_sqlite
 * Execute a query on a sqlite connection, set the returned values in the json result
 * Should not be executed by the user because all parameters are supposed to be correct
 * return H_OK on success
 */
int h_execute_query_json_sqlite(const struct _h_connection * conn, const char * query, json_t ** j_result) {
  sqlite3_stmt *stmt;
  int sql_result, row_result, nb_columns, col;
  json_t * j_data;
  
  if (j_result == NULL) {
    return H_ERROR_PARAMS;
  }
  
  sql_result = sqlite3_prepare_v2(((struct _h_sqlite *)conn->connection)->db_handle, query, strlen(query)+1, &stmt, NULL);
  
  if (sql_result == SQLITE_OK) {
    nb_columns = sqlite3_column_count(stmt);
    /* Filling j_result with results in json format */
    *j_result = json_array();
    if (*j_result == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Hoel - Error allocating memory for *j_result");
      sqlite3_finalize(stmt);
      return H_ERROR_MEMORY;
    }
    row_result = sqlite3_step(stmt);
    while (row_result == SQLITE_ROW) {
      j_data = json_object();
      if (j_data == NULL) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Hoel - Error allocating memory for j_data");
        json_decref(*j_result);
        return H_ERROR_MEMORY;
      }
      for (col = 0; col < nb_columns; col++) {
        switch (sqlite3_column_type(stmt, col)) {
          case SQLITE_INTEGER:
            json_object_set_new(j_data, sqlite3_column_name(stmt, col), json_integer(sqlite3_column_int(stmt, col)));
            break;
          case SQLITE_FLOAT:
            json_object_set_new(j_data, sqlite3_column_name(stmt, col), json_real(sqlite3_column_double(stmt, col)));
            break;
          case SQLITE_BLOB:
            json_object_set_new(j_data, sqlite3_column_name(stmt, col), json_stringn(sqlite3_column_blob(stmt, col), sqlite3_column_bytes(stmt, col)));
            break;
          case SQLITE3_TEXT:
            json_object_set_new(j_data, sqlite3_column_name(stmt, col), json_string((char*)sqlite3_column_text(stmt, col)));
            break;
          case SQLITE_NULL:
            json_object_set_new(j_data, sqlite3_column_name(stmt, col), json_null());
          default:
            break;
        }
      }
      json_array_append_new(*j_result, j_data);
      j_data = NULL;
      row_result = sqlite3_step(stmt);
    }
    sqlite3_finalize(stmt);
    return H_OK;
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error executing sql query");
    y_log_message(Y_LOG_LEVEL_DEBUG, "Error code: %d, message: \"%s\"", 
                                   sqlite3_errcode(((struct _h_sqlite *)conn->connection)->db_handle), 
                                   sqlite3_errmsg(((struct _h_sqlite *)conn->connection)->db_handle));
    y_log_message(Y_LOG_LEVEL_DEBUG, "Query: \"%s\"", query);
    sqlite3_finalize(stmt);
    return H_ERROR_QUERY;
  }
}
#else

/**
 * Dummy functions when Hoel is not built with SQLite
 */
struct _h_connection * h_connect_sqlite(const char * db_path) {
  UNUSED(db_path);
	y_log_message(Y_LOG_LEVEL_ERROR, "Hoel was not compiled with SQLite backend");
	return NULL;
}

void h_close_sqlite(struct _h_connection * conn) {
  UNUSED(conn);
	y_log_message(Y_LOG_LEVEL_ERROR, "Hoel was not compiled with SQLite backend");
}

#endif
