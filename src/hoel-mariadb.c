/**
 * 
 * Hoel database abstraction library
 * 
 * hoel-mariadb.c: Maria DB/Mysql specific functions
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
#include "hoel.h"

#ifdef _HOEL_MARIADB
// MariaDB library Includes
#include <my_global.h>
#include <mysql.h>

/**
 * MariaDB handle
 */
struct _h_mariadb {
  char * host;
  char * user;
  char * passwd;
  char * db;
  unsigned int port;
  char * unix_socket;
  unsigned long flags;
  MYSQL * db_handle;
};

/**
 * h_connect_mariadb
 * Opens a database connection to a mariadb server
 * return pointer to a struct _h_connection * on sucess, NULL on error
 */
struct _h_connection * h_connect_mariadb(char * host, char * user, char * passwd, char * db, unsigned int port, char * unix_socket) {
  struct _h_connection * conn = NULL;
  if (host != NULL && db != NULL) {
    conn = malloc(sizeof(struct _h_connection));
    if (conn == NULL) {
      return NULL;
    }
    
    conn->type = HOEL_DB_TYPE_MARIADB;
    conn->connection = malloc(sizeof(struct _h_mariadb));
    if (conn->connection == NULL) {
      free(conn);
      return NULL;
    }
    ((struct _h_mariadb *)conn->connection)->db_handle = mysql_init(NULL);
    if (((struct _h_mariadb *)conn->connection)->db_handle == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "mysql_init error, aborting");
      return NULL;
    }
    if (mysql_real_connect(((struct _h_mariadb *)conn->connection)->db_handle,
                           host, user, passwd, db, port, unix_socket, CLIENT_COMPRESS) == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error connecting to mariadb database %s", db);
      y_log_message(Y_LOG_LEVEL_DEBUG, "Error message: \"%s\"", mysql_error(((struct _h_mariadb *)conn->connection)->db_handle));
      mysql_close(((struct _h_mariadb *)conn->connection)->db_handle);
      return NULL;
    } else {
      return conn;
    }
  }
  return conn;
}

/**
 * close connection to database
 */
void h_close_mariadb(struct _h_connection * conn) {
  mysql_close(((struct _h_mariadb *)conn->connection)->db_handle);
  mysql_library_end();
}

/**
 * escape a string
 * returned value must be free'd after use
 */
char * h_escape_string_mariadb(const struct _h_connection * conn, const char * unsafe) {
  char * escaped = malloc(2 * strlen(unsafe) + sizeof(char));
  if (escaped == NULL) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error escaping string \"%s\"", unsafe);
    y_log_message(Y_LOG_LEVEL_DEBUG, "Error message: \"%s\"", mysql_error(((struct _h_mariadb *)conn->connection)->db_handle));
    return NULL;
  }
  mysql_real_escape_string(((struct _h_mariadb *)conn->connection)->db_handle, escaped, unsafe, strlen(unsafe));
  return escaped;
}

/**
 * Return the id of the last inserted value
 */
int h_last_insert_id_mariadb(const struct _h_connection * conn) {
  int id = mysql_insert_id(((struct _h_mariadb *)conn->connection)->db_handle);
  if (id <= 0) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error executing mysql_insert_id");
    y_log_message(Y_LOG_LEVEL_DEBUG, "Error message: \"%s\"", mysql_error(((struct _h_mariadb *)conn->connection)->db_handle));
  }
  return id;
}

/**
 * h_execute_query_mariadb
 * Execute a query on a mariadb connection, set the result structure with the returned values
 * Should not be executed by the user because all parameters are supposed to be correct
 * if result is NULL, the query is executed but no value will be returned
 * return H_OK on success
 */
int h_execute_query_mariadb(const struct _h_connection * conn, const char * query, struct _h_result * h_result) {
  MYSQL_RES * result;
  uint num_fields, col, row;
  MYSQL_ROW m_row;
  MYSQL_FIELD * fields;
  struct _h_data * data, * cur_row = NULL;
  unsigned long * lengths;
  int res;
  
  if (mysql_query(((struct _h_mariadb *)conn->connection)->db_handle, query)) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error executing sql query");
    y_log_message(Y_LOG_LEVEL_DEBUG, "Error message: \"%s\"", mysql_error(((struct _h_mariadb *)conn->connection)->db_handle));
    y_log_message(Y_LOG_LEVEL_DEBUG, "Query: \"%s\"", query);
    return H_ERROR_QUERY;
  }
  
  if (h_result != NULL) {
    result = mysql_store_result(((struct _h_mariadb *)conn->connection)->db_handle);
    
    if (result == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error executing mysql_store_result");
      y_log_message(Y_LOG_LEVEL_DEBUG, "Error message: \"%s\"", mysql_error(((struct _h_mariadb *)conn->connection)->db_handle));
      return H_ERROR_QUERY;
    }
    
    num_fields = mysql_num_fields(result);
    fields = mysql_fetch_fields(result);
    
    h_result->nb_rows = 0;
    h_result->nb_columns = num_fields;
    h_result->data = NULL;
    for (row = 0; (m_row = mysql_fetch_row(result)) != NULL; row++) {
      cur_row = NULL;
      lengths = mysql_fetch_lengths(result);
      for (col=0; col<num_fields; col++) {
        data = h_get_mariadb_value(m_row[col], lengths[col], fields[col].type);
        res = h_row_add_data(&cur_row, data, col);
        h_clean_data_full(data);
        if (res != H_OK) {
          mysql_free_result(result);
          return res;
        }
      }
      res = h_result_add_row(h_result, cur_row, row);
      if (res != H_OK) {
        mysql_free_result(result);
        return res;
      }
    }
    mysql_free_result(result);
  }
  
  return H_OK;
}

/**
 * h_execute_query_json_mariadb
 * Execute a query on a mariadb connection, set the returned values in the json result
 * Should not be executed by the user because all parameters are supposed to be correct
 * return H_OK on success
 */
int h_execute_query_json_mariadb(const struct _h_connection * conn, const char * query, json_t ** j_result) {
  MYSQL_RES * result;
  uint num_fields, col, row;
  MYSQL_ROW m_row;
  MYSQL_FIELD * fields;
  unsigned long * lengths;
  json_t * j_data;
  struct _h_data * h_data;
  char date_stamp[20];
  
  if (j_result == NULL) {
    return H_ERROR_PARAMS;
  }
  
  *j_result = json_array();
  if (*j_result == NULL) {
    return H_ERROR_MEMORY;
  }

  if (mysql_query(((struct _h_mariadb *)conn->connection)->db_handle, query)) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error executing sql query");
    y_log_message(Y_LOG_LEVEL_DEBUG, "Error message: \"%s\"", mysql_error(((struct _h_mariadb *)conn->connection)->db_handle));
    y_log_message(Y_LOG_LEVEL_DEBUG, "Query: \"%s\"", query);
    return H_ERROR_QUERY;
  }
  
  result = mysql_store_result(((struct _h_mariadb *)conn->connection)->db_handle);
  
  if (result == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error executing mysql_store_result");
      y_log_message(Y_LOG_LEVEL_DEBUG, "Error message: \"%s\"", mysql_error(((struct _h_mariadb *)conn->connection)->db_handle));
    return H_ERROR_QUERY;
  }
  
  num_fields = mysql_num_fields(result);
  fields = mysql_fetch_fields(result);
  
  for (row = 0; (m_row = mysql_fetch_row(result)) != NULL; row++) {
    j_data = json_object();
    if (j_data == NULL) {
      json_decref(*j_result);
      return H_ERROR_MEMORY;
    }
    lengths = mysql_fetch_lengths(result);
    for (col=0; col<num_fields; col++) {
      h_data = h_get_mariadb_value(m_row[col], lengths[col], fields[col].type);
      switch (h_data->type) {
        case HOEL_COL_TYPE_INT:
          json_object_set_new(j_data, fields[col].name, json_integer(((struct _h_type_int *)h_data->t_data)->value));
          break;
        case HOEL_COL_TYPE_DOUBLE:
          json_object_set_new(j_data, fields[col].name, json_real(((struct _h_type_double *)h_data->t_data)->value));
          break;
        case HOEL_COL_TYPE_TEXT:
          json_object_set_new(j_data, fields[col].name, json_string(((struct _h_type_text *)h_data->t_data)->value));
          break;
        case HOEL_COL_TYPE_DATE:
          strftime (date_stamp, sizeof(date_stamp), "%FT%TZ", &((struct _h_type_datetime *)h_data->t_data)->value);
          json_object_set_new(j_data, fields[col].name, json_string(date_stamp));
          break;
        case HOEL_COL_TYPE_BLOB:
          json_object_set_new(j_data, fields[col].name, json_stringn(((struct _h_type_blob *)h_data->t_data)->value, ((struct _h_type_blob *)h_data->t_data)->length));
          break;
        case HOEL_COL_TYPE_NULL:
          json_object_set_new(j_data, fields[col].name, json_null());
          break;
      }
      h_clean_data_full(h_data);
    }
    json_array_append_new(*j_result, j_data);
    j_data = NULL;
  }
  mysql_free_result(result);
  
  return H_OK;
}

/**
 * h_get_mariadb_value
 * convert value into a struct _h_data * depening on the m_type given
 * returned value must be free'd with h_clean_data_full after use
 */
struct _h_data * h_get_mariadb_value(const char * value, const unsigned long length, const int m_type) {
  struct _h_data * data = NULL;
  int i_value;
  double d_value;
  struct tm tm_value;
  char * endptr;
  
  if (value != NULL) {
    switch (m_type) {
      case FIELD_TYPE_DECIMAL:
      case FIELD_TYPE_NEWDECIMAL:
      case FIELD_TYPE_TINY:
      case FIELD_TYPE_SHORT:
      case FIELD_TYPE_LONG:
      case FIELD_TYPE_LONGLONG:
      case FIELD_TYPE_INT24:
      case FIELD_TYPE_YEAR:
        i_value = strtol(value, &endptr, 10);
        if (endptr != value) {
          data = h_new_data_int(i_value);
        } else {
          data = h_new_data_null();
        }
        break;
      case FIELD_TYPE_BIT:
        i_value = strtol(value, &endptr, 2);
        if (endptr != value) {
          data = h_new_data_int(i_value);
        } else {
          data = h_new_data_null();
        }
        break;
      case FIELD_TYPE_FLOAT:
      case FIELD_TYPE_DOUBLE:
        d_value = strtod(value, &endptr);
        if (endptr != value) {
          data = h_new_data_double(d_value);
        } else {
          data = h_new_data_null();
        }
        break;
      case FIELD_TYPE_NULL:
        data = h_new_data_null();
        break;
      case FIELD_TYPE_DATE:
        if (strptime(value, "%F", &tm_value) == NULL) {
          data = h_new_data_null();
        } else {
          data = h_new_data_datetime(&tm_value);
        }
        break;
      case FIELD_TYPE_TIME:
        if (strptime(value, "%T", &tm_value) == NULL) {
          data = h_new_data_null();
        } else {
          data = h_new_data_datetime(&tm_value);
        }
        break;
      case FIELD_TYPE_TIMESTAMP:
      case FIELD_TYPE_DATETIME:
      case FIELD_TYPE_NEWDATE:
        if (strptime(value, "%F %T", &tm_value) == NULL) {
          data = h_new_data_null();
        } else {
          data = h_new_data_datetime(&tm_value);
        }
        break;
      case FIELD_TYPE_TINY_BLOB:
      case FIELD_TYPE_MEDIUM_BLOB:
      case FIELD_TYPE_LONG_BLOB:
      case FIELD_TYPE_BLOB:
        if (length > 0) {
          data = h_new_data_blob(value, length);
        } else {
          data = h_new_data_null();
        }
        break;
      case FIELD_TYPE_VAR_STRING:
      case FIELD_TYPE_ENUM:
      case FIELD_TYPE_SET:
      case FIELD_TYPE_GEOMETRY:
      default:
        data = h_new_data_text(value);
        break;
    }
  } else {
    data = h_new_data_null();
  }
  return data;
}
#endif
