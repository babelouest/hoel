/**
 * 
 * Hoel database abstraction library
 * 
 * hoel-mariadb.c: Maria DB/Mysql specific functions
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

#ifdef _HOEL_MARIADB
/* MariaDB library Includes */
#include <my_global.h>
#include <mysql.h>
#include <string.h>

#ifndef HOEL_MARIADB_POOL_SIZE
  #define HOEL_MARIADB_POOL_SIZE 3
#endif

/**
 * MariaDB handle
 */
struct _h_mariadb {
  MYSQL ** db_handle;
  unsigned short int * db_handle_lock;
  unsigned int pool_size;
  unsigned int nb_handle_lock;
  pthread_cond_t  pool_message;
  pthread_mutex_t pool_lock;
  pthread_mutex_t handle_lock;
};

/**
 * h_connect_mariadb_pool_size
 * Opens a database connection to a mariadb server with a pool size specified
 * return pointer to a struct _h_connection * on sucess, NULL on error
 */
struct _h_connection * h_connect_mariadb_pool_size(const char * host, const char * user, const char * passwd, const char * db, const unsigned int port, const char * unix_socket, unsigned int pool_size) {
  struct _h_connection * conn = NULL;
  my_bool reconnect = 1;
  unsigned int i, has_error = 0;
  pthread_mutexattr_t mutexattr;

  if (host != NULL && db != NULL && pool_size) {
    conn = o_malloc(sizeof(struct _h_connection));
    if (conn == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Hoel - Error allocating memory for conn");
      return NULL;
    }
    
    conn->type = HOEL_DB_TYPE_MARIADB;
    conn->connection = o_malloc(sizeof(struct _h_mariadb));
    if (conn->connection == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Hoel - Error allocating memory for conn->connection");
      has_error = 1;
    } else {
      ((struct _h_mariadb *)conn->connection)->pool_size = pool_size;
      ((struct _h_mariadb *)conn->connection)->db_handle_lock = NULL;
      ((struct _h_mariadb *)conn->connection)->db_handle = NULL;
      if (mysql_library_init(0, NULL, NULL)) {
        y_log_message(Y_LOG_LEVEL_ERROR, "mysql_library_init error, aborting");
        has_error = 1;
      } else {
        ((struct _h_mariadb *)conn->connection)->db_handle_lock = o_malloc(pool_size * sizeof(int));
        if (((struct _h_mariadb *)conn->connection)->db_handle_lock == NULL) {
          y_log_message(Y_LOG_LEVEL_ERROR, "db_handle_lock allocation error, aborting");
          has_error = 1;
        } else {
          ((struct _h_mariadb *)conn->connection)->db_handle = o_malloc(pool_size * sizeof(MYSQL *));
          if (((struct _h_mariadb *)conn->connection)->db_handle == NULL) {
            y_log_message(Y_LOG_LEVEL_ERROR, "db_handle allocation error, aborting");
            has_error = 1;
          } else {
            for (i=0; !has_error && i<pool_size; i++) {
              ((struct _h_mariadb *)conn->connection)->db_handle_lock[i] = 0;
              ((struct _h_mariadb *)conn->connection)->db_handle[i] = mysql_init(NULL);
              if (((struct _h_mariadb *)conn->connection)->db_handle[i] == NULL) {
                y_log_message(Y_LOG_LEVEL_ERROR, "mysql_init error, aborting");
                has_error = 1;
              }
              if (mysql_real_connect(((struct _h_mariadb *)conn->connection)->db_handle[i],
                                     host, user, passwd, db, port, unix_socket, CLIENT_COMPRESS) == NULL) {
                y_log_message(Y_LOG_LEVEL_ERROR, "Error connecting to mariadb database %s", db);
                y_log_message(Y_LOG_LEVEL_DEBUG, "Error message: \"%s\"", mysql_error(((struct _h_mariadb *)conn->connection)->db_handle[i]));
                mysql_close(((struct _h_mariadb *)conn->connection)->db_handle[i]);
                has_error = 1;
              } else {
                /* Set MYSQL_OPT_RECONNECT to true to reconnect automatically when connection is closed by the server (to avoid CR_SERVER_GONE_ERROR) */
                mysql_options(((struct _h_mariadb *)conn->connection)->db_handle[i], MYSQL_OPT_RECONNECT, &reconnect);
              }
            }
          }
        }
      }
    }
    if (has_error) {
      if (conn->connection != NULL) {
        for (i=0; i<pool_size; i++) {
          mysql_close(((struct _h_mariadb *)conn->connection)->db_handle[i]);
        }
        o_free(((struct _h_mariadb *)conn->connection)->db_handle);
        o_free(((struct _h_mariadb *)conn->connection)->db_handle_lock);
        o_free(conn->connection);
      }
      o_free(conn);
      conn = NULL;
    }
    /* Initialize MUTEX for connection */
    ((struct _h_mariadb *)conn->connection)->nb_handle_lock = 0;
    pthread_mutexattr_init ( &mutexattr );
    pthread_mutexattr_settype( &mutexattr, PTHREAD_MUTEX_RECURSIVE );
    if (!has_error && (pthread_mutex_init(&((struct _h_mariadb *)conn->connection)->pool_lock, NULL) || pthread_cond_init(&((struct _h_mariadb *)conn->connection)->pool_message, NULL) || pthread_mutex_init(&(((struct _h_mariadb *)conn->connection)->handle_lock), &mutexattr))) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Impossible to initialize Mutex pool_lock for MariaDB connection");
    }
  }
  return conn;
}

/**
 * h_connect_mariadb
 * Opens a database connection to a mariadb server with a default pool size
 * return pointer to a struct _h_connection * on sucess, NULL on error
 */
struct _h_connection * h_connect_mariadb(const char * host, const char * user, const char * passwd, const char * db, const unsigned int port, const char * unix_socket) {
  return h_connect_mariadb_pool_size(host, user, passwd, db, port, unix_socket, HOEL_MARIADB_POOL_SIZE);
}

/**
 * close connection to database
 */
void h_close_mariadb(struct _h_connection * conn) {
  unsigned int i;
  if (conn->connection != NULL) {
    for (i=0; i<((struct _h_mariadb *)conn->connection)->pool_size; i++) {
      mysql_close(((struct _h_mariadb *)conn->connection)->db_handle[i]);
    }
    pthread_mutex_destroy(&((struct _h_mariadb *)conn->connection)->pool_lock);
    pthread_mutex_destroy(&((struct _h_mariadb *)conn->connection)->handle_lock);
    o_free(((struct _h_mariadb *)conn->connection)->db_handle);
    o_free(((struct _h_mariadb *)conn->connection)->db_handle_lock);
  }
  mysql_library_end();
}

/**
 * Get the first free db_handle
 */
static MYSQL * h_get_handle_mariadb(const struct _h_connection * conn) {
  unsigned int i;
  MYSQL * ret = NULL;
  unsigned int pool_ticket;
  
  if (pthread_mutex_lock(&(((struct _h_mariadb *)conn->connection)->handle_lock))) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Hoel/h_get_handle_mariadb - Error pthread_mutex_lock");
  } else {
    pool_ticket = (((struct _h_mariadb *)conn->connection)->nb_handle_lock++);
    pthread_mutex_unlock(&(((struct _h_mariadb *)conn->connection)->handle_lock));
    
    //y_log_message(Y_LOG_LEVEL_DEBUG, "Enter h_get_handle_mariadb, pool_size is %d", ((struct _h_mariadb *)conn->connection)->nb_handle_lock);
    while (pool_ticket >= ((struct _h_mariadb *)conn->connection)->pool_size) {
      pthread_mutex_lock(&((struct _h_mariadb *)conn->connection)->pool_lock);
      pthread_cond_wait(&((struct _h_mariadb *)conn->connection)->pool_message, &((struct _h_mariadb *)conn->connection)->pool_lock);
      pthread_mutex_unlock(&((struct _h_mariadb *)conn->connection)->pool_lock);
      //y_log_message(Y_LOG_LEVEL_DEBUG, "On lock released, pool_size is %d", ((struct _h_mariadb *)conn->connection)->nb_handle_lock);
      pool_ticket--;
    }
    
    /* At least one db_handle should be free, lock it */
    if (pthread_mutex_lock(&(((struct _h_mariadb *)conn->connection)->handle_lock))) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Hoel/h_get_handle_mariadb - Error pthread_mutex_lock");
    } else {
      for (i=0; i<((struct _h_mariadb *)conn->connection)->pool_size; i++) {
        if (!((struct _h_mariadb *)conn->connection)->db_handle_lock[i]) {
          ((struct _h_mariadb *)conn->connection)->db_handle_lock[i] = 1;
          ret = ((struct _h_mariadb *)conn->connection)->db_handle[i];
          break;
        }
      }
      pthread_mutex_unlock(&(((struct _h_mariadb *)conn->connection)->handle_lock));
    }
    //y_log_message(Y_LOG_LEVEL_DEBUG, "Exit h_get_handle_mariadb, lock at index %d", i);
  }
  
  return ret;
}

/**
 * Release the db_handle
 */
static void h_release_handle_mariadb(const struct _h_connection * conn, MYSQL * db_handle) {
  unsigned int i = 0;
  
  //y_log_message(Y_LOG_LEVEL_DEBUG, "Enter h_release_handle_mariadb");
  if (pthread_mutex_lock(&(((struct _h_mariadb *)conn->connection)->handle_lock))) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Hoel/h_release_handle_mariadb - Error pthread_mutex_lock");
  } else {
    for (i=0; i<((struct _h_mariadb *)conn->connection)->pool_size; i++) {
      if (((struct _h_mariadb *)conn->connection)->db_handle[i] == db_handle) {
        //y_log_message(Y_LOG_LEVEL_DEBUG, "Release lock at index %d", i);
        ((struct _h_mariadb *)conn->connection)->db_handle_lock[i] = 0;
        ((struct _h_mariadb *)conn->connection)->nb_handle_lock--;
        pthread_mutex_lock(&((struct _h_mariadb *)conn->connection)->pool_lock);
        pthread_cond_broadcast(&((struct _h_mariadb *)conn->connection)->pool_message);
        pthread_mutex_unlock(&((struct _h_mariadb *)conn->connection)->pool_lock);
        break;
      }
    }
    pthread_mutex_unlock(&(((struct _h_mariadb *)conn->connection)->handle_lock));
  }
  if (i >= ((struct _h_mariadb *)conn->connection)->pool_size) {
    pthread_mutex_lock(&((struct _h_mariadb *)conn->connection)->pool_lock);
    pthread_cond_broadcast(&((struct _h_mariadb *)conn->connection)->pool_message);
    pthread_mutex_unlock(&((struct _h_mariadb *)conn->connection)->pool_lock);
    y_log_message(Y_LOG_LEVEL_ERROR, "Hoel/h_release_handle_mariadb - Error release db_handle, not found in list");
  }
  //y_log_message(Y_LOG_LEVEL_DEBUG, "Exit h_release_handle_mariadb");
}

/**
 * escape a string
 * returned value must be free'd after use
 */
char * h_escape_string_mariadb(const struct _h_connection * conn, const char * unsafe) {
  char * escaped = o_malloc(2 * strlen(unsafe) + sizeof(char));
  MYSQL * db_handle = h_get_handle_mariadb(conn);
  if (db_handle != NULL) {
    if (escaped == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Hoel - Error allocating memory for escaped");
      return NULL;
    }
    mysql_real_escape_string(db_handle, escaped, unsafe, strlen(unsafe));
    h_release_handle_mariadb(conn, db_handle);
  }
  return escaped;
}

/**
 * Return the id of the last inserted value
 */
int h_last_insert_id_mariadb(const struct _h_connection * conn) {
  int id = 0;
  MYSQL * db_handle = h_get_handle_mariadb(conn);
  if (db_handle != NULL) {
    id = mysql_insert_id(db_handle);
    if (id <= 0) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error executing mysql_insert_id");
      y_log_message(Y_LOG_LEVEL_DEBUG, "Error message: \"%s\"", mysql_error(db_handle));
    }
    h_release_handle_mariadb(conn, db_handle);
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
  
  MYSQL * db_handle = h_get_handle_mariadb(conn);
  if (db_handle != NULL) {
    if (mysql_query(db_handle, query)) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error executing sql query");
      y_log_message(Y_LOG_LEVEL_DEBUG, "Error message: \"%s\"", mysql_error(db_handle));
      y_log_message(Y_LOG_LEVEL_DEBUG, "Query: \"%s\"", query);
      return H_ERROR_QUERY;
    }
    
    if (h_result != NULL) {
      result = mysql_store_result(db_handle);
      
      if (result == NULL) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Error executing mysql_store_result");
        y_log_message(Y_LOG_LEVEL_DEBUG, "Error message: \"%s\"", mysql_error(db_handle));
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
    h_release_handle_mariadb(conn, db_handle);
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
  
  MYSQL * db_handle = h_get_handle_mariadb(conn);
  if (db_handle != NULL) {
    if (j_result == NULL) {
      return H_ERROR_PARAMS;
    }
    
    *j_result = json_array();
    if (*j_result == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Hoel - Error allocating memory for *j_result");
      return H_ERROR_MEMORY;
    }

    if (mysql_query(db_handle, query)) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error executing sql query");
      y_log_message(Y_LOG_LEVEL_DEBUG, "Error message: \"%s\"", mysql_error(db_handle));
      y_log_message(Y_LOG_LEVEL_DEBUG, "Query: \"%s\"", query);
      json_decref(*j_result);
      return H_ERROR_QUERY;
    }
    
    result = mysql_store_result(db_handle);
    
    if (result == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error executing mysql_store_result");
      y_log_message(Y_LOG_LEVEL_DEBUG, "Error message: \"%s\"", mysql_error(db_handle));
      json_decref(*j_result);
      return H_ERROR_QUERY;
    }
    
    num_fields = mysql_num_fields(result);
    fields = mysql_fetch_fields(result);
    
    for (row = 0; (m_row = mysql_fetch_row(result)) != NULL; row++) {
      j_data = json_object();
      if (j_data == NULL) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Hoel - Error allocating memory for j_data");
        json_decref(*j_result);
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
    h_release_handle_mariadb(conn, db_handle);
  }
  
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

#else

/**
 * Dummy functions when Hoel is not built with MariaDB
 */
struct _h_connection * h_connect_mariadb(const char * host, const char * user, const char * passwd, const char * db, const unsigned int port, const char * unix_socket) {
  UNUSED(host);
  UNUSED(user);
  UNUSED(passwd);
  UNUSED(db);
  UNUSED(port);
  UNUSED(unix_socket);
  y_log_message(Y_LOG_LEVEL_ERROR, "Hoel was not compiled with MariaDB backend");
  return NULL;
}

struct _h_connection * h_connect_mariadb_sized(const char * host, const char * user, const char * passwd, const char * db, const unsigned int port, const char * unix_socket, unsigned int pool_size) {
  UNUSED(host);
  UNUSED(user);
  UNUSED(passwd);
  UNUSED(db);
  UNUSED(port);
  UNUSED(unix_socket);
  UNUSED(pool_size);
  y_log_message(Y_LOG_LEVEL_ERROR, "Hoel was not compiled with MariaDB backend");
  return NULL;
}

void h_close_mariadb(struct _h_connection * conn) {
  UNUSED(conn);
  y_log_message(Y_LOG_LEVEL_ERROR, "Hoel was not compiled with MariaDB backend");
}

#endif
