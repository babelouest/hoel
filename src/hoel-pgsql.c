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
/* PostgreSQL library includes */
#include <libpq-fe.h>
#include <string.h>

struct _h_pg_type {
  Oid            pg_type;
  unsigned short h_type;
};

/**
 * Postgre SQL handle
 */
struct _h_pgsql {
  char              * conninfo;
  PGconn            * db_handle;
  unsigned int        nb_type;
  struct _h_pg_type * list_type;
  pthread_mutex_t     lock;
};

/**
 * h_connect_pgsql
 * Opens a database connection to a PostgreSQL server
 * return pointer to a struct _h_connection * on sucess, NULL on error
 */
struct _h_connection * h_connect_pgsql(const char * conninfo) {
  struct _h_connection * conn = NULL;
  int ntuples, i;
  PGresult *res;
  pthread_mutexattr_t mutexattr;
  
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
    ((struct _h_pgsql *)conn->connection)->nb_type = 0;
    ((struct _h_pgsql *)conn->connection)->list_type = NULL;
    
    if (PQstatus(((struct _h_pgsql *)conn->connection)->db_handle) != CONNECTION_OK) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Hoel - Error connecting to PostgreSQL Database");
      y_log_message(Y_LOG_LEVEL_DEBUG, "Hoel - Error message: \"%s\"", PQerrorMessage(((struct _h_pgsql *)conn->connection)->db_handle));
      PQfinish(((struct _h_pgsql *)conn->connection)->db_handle);
      free(conn->connection);
      free(conn);
      conn = NULL;
    } else {
      res = PQexec(((struct _h_pgsql *)conn->connection)->db_handle, "select oid, typname from pg_type");
      if (PQresultStatus(res) != PGRES_TUPLES_OK && PQresultStatus(res) != PGRES_COMMAND_OK && PQnfields(res) == 2) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Error executing sql query");
        y_log_message(Y_LOG_LEVEL_DEBUG, "Error message: \"%s\"", PQerrorMessage(((struct _h_pgsql *)conn->connection)->db_handle));
        y_log_message(Y_LOG_LEVEL_DEBUG, "Query: \"select oid, typname from pg_type\"");
        PQfinish(((struct _h_pgsql *)conn->connection)->db_handle);
        free(conn->connection);
        free(conn);
        conn = NULL;
      } else {
        ntuples = PQntuples(res);
        ((struct _h_pgsql *)conn->connection)->list_type = o_malloc((ntuples+1)*sizeof(struct _h_pg_type));
        if (((struct _h_pgsql *)conn->connection)->list_type != NULL) {
          ((struct _h_pgsql *)conn->connection)->nb_type = ntuples;
          for(i = 0; i < ntuples; i++) {
            char * cur_type_name = PQgetvalue(res, i, 1);
            ((struct _h_pgsql *)conn->connection)->list_type[i].pg_type = strtol(PQgetvalue(res, i, 0), NULL, 10);
            if (o_strcmp(cur_type_name, "bool") == 0) {
              ((struct _h_pgsql *)conn->connection)->list_type[i].h_type = HOEL_COL_TYPE_BOOL;
            } else if (o_strncmp(cur_type_name, "int", 3) == 0 || (o_strncmp(cur_type_name+1, "id", 2) == 0 && o_strlen(cur_type_name) == 3)) {
              ((struct _h_pgsql *)conn->connection)->list_type[i].h_type = HOEL_COL_TYPE_INT;
            } else if (o_strcmp(cur_type_name, "numeric") == 0 || o_strncmp(cur_type_name, "float", 5) == 0) {
              ((struct _h_pgsql *)conn->connection)->list_type[i].h_type = HOEL_COL_TYPE_DOUBLE;
            } else if (o_strcmp(cur_type_name, "date") == 0 || o_strncmp(cur_type_name, "time", 4) == 0) {
              ((struct _h_pgsql *)conn->connection)->list_type[i].h_type = HOEL_COL_TYPE_DATE;
            } else if (o_strcmp(cur_type_name, "bytea") == 0) {
              ((struct _h_pgsql *)conn->connection)->list_type[i].h_type = HOEL_COL_TYPE_BLOB;
            } else if (o_strcmp(cur_type_name, "bool") == 0) {
              ((struct _h_pgsql *)conn->connection)->list_type[i].h_type = HOEL_COL_TYPE_BOOL;
            } else {
              ((struct _h_pgsql *)conn->connection)->list_type[i].h_type = HOEL_COL_TYPE_TEXT;
            }
          }
          /* Initialize MUTEX for connection */
          pthread_mutexattr_init ( &mutexattr );
          pthread_mutexattr_settype( &mutexattr, PTHREAD_MUTEX_RECURSIVE );
          if (pthread_mutex_init(&(((struct _h_pgsql *)conn->connection)->lock), &mutexattr) != 0) {
            y_log_message(Y_LOG_LEVEL_ERROR, "Impossible to initialize Mutex Lock for MariaDB connection");
          }
          pthread_mutexattr_destroy( &mutexattr );
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "Error allocating resources for list_type");
          PQfinish(((struct _h_pgsql *)conn->connection)->db_handle);
          free(conn->connection);
          free(conn);
          conn = NULL;
        }
        PQclear(res);
      }
    }
  }
  return conn;
}

/**
 * close a pgsql connection
 */
void h_close_pgsql(struct _h_connection * conn) {
  PQfinish(((struct _h_pgsql *)conn->connection)->db_handle);
  o_free(((struct _h_pgsql *)conn->connection)->list_type);
  ((struct _h_pgsql *)conn->connection)->list_type = NULL;
  ((struct _h_pgsql *)conn->connection)->nb_type = 0;
  pthread_mutex_destroy(&((struct _h_pgsql *)conn->connection)->lock);
}

/**
 * escape a string
 * returned value must be free'd after use
 */
char * h_escape_string_pgsql(const struct _h_connection * conn, const char * unsafe) {
  char * escaped = PQescapeLiteral(((struct _h_pgsql *)conn->connection)->db_handle, unsafe, strlen(unsafe)), * to_return = NULL;
  if (escaped != NULL) {
    if (escaped[0] == '\'' && escaped[o_strlen(escaped)-1] == '\'') {
      to_return = o_strndup((escaped+1), o_strlen((escaped+1))-1);
    }
    PQfreemem(escaped);
  }
  return to_return;
}

/**
 * Escapes a string and returns it ready to be inserted in the query
 * returned value must be free'd after use
 */
char * h_escape_string_with_quotes_pgsql(const struct _h_connection * conn, const char * unsafe) {
  char * escaped = PQescapeLiteral(((struct _h_pgsql *)conn->connection)->db_handle, unsafe, strlen(unsafe)), * to_return = NULL;
  if (escaped != NULL) {
    to_return = o_strdup(escaped);
    PQfreemem(escaped);
  }
  return to_return;
}

/**
 * Return the hoel type of a column given its Oid
 * If type is not found, return HOEL_COL_TYPE_TEXT
 */
static unsigned short h_get_type_from_oid(const struct _h_connection * conn, Oid pg_type) {
  unsigned int i;
  
  for (i = 0; i < ((struct _h_pgsql *)conn->connection)->nb_type; i++) {
    if (((struct _h_pgsql *)conn->connection)->list_type[i].pg_type == pg_type) {
      return ((struct _h_pgsql *)conn->connection)->list_type[i].h_type;
    }
  }
  pthread_mutex_unlock(&(((struct _h_pgsql *)conn->connection)->lock));
  return HOEL_COL_TYPE_TEXT;
}

/**
 * h_execute_query_pgsql
 * Execute a query on a pgsql connection, set the result structure with the returned values
 * Should not be executed by the user because all parameters are supposed to be correct
 * if result is NULL, the query is executed but no value will be returned
 * return H_OK on success
 */
int h_execute_query_pgsql(const struct _h_connection * conn, const char * query, struct _h_result * result) {
  PGresult * res;
  int nfields, ntuples, i, j, h_res, ret = H_OK;
  struct _h_data * data, * cur_row = NULL;
  
  if (pthread_mutex_lock(&(((struct _h_pgsql *)conn->connection)->lock))) {
    ret = H_ERROR_QUERY;
  } else {
    res = PQexec(((struct _h_pgsql *)conn->connection)->db_handle, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK && PQresultStatus(res) != PGRES_COMMAND_OK) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error executing sql query");
      y_log_message(Y_LOG_LEVEL_DEBUG, "Error message: \"%s\"", PQerrorMessage(((struct _h_pgsql *)conn->connection)->db_handle));
      y_log_message(Y_LOG_LEVEL_DEBUG, "Query: \"%s\"", query);
      ret = H_ERROR_QUERY;
    } else {
      nfields = PQnfields(res);
      ntuples = PQntuples(res);

      if (result != NULL) {
        result->nb_rows = 0;
        result->nb_columns = nfields;
        result->data = NULL;
        for(i = 0; ret == H_OK && i < ntuples; i++) {
          cur_row = NULL;
          for(j = 0; ret == H_OK && j < nfields; j++) {
            char * val = PQgetvalue(res, i, j);
            if (val == NULL) {
              data = h_new_data_null();
            } else {
              switch (h_get_type_from_oid(conn, PQftype(res, j))) {
                case HOEL_COL_TYPE_INT:
                  data = h_new_data_int(strtol(val, NULL, 10));
                  break;
                case HOEL_COL_TYPE_DOUBLE:
                  data = h_new_data_double(strtod(val, NULL));
                  break;
                case HOEL_COL_TYPE_BLOB:
                  data = h_new_data_blob(val, PQfsize(res, i));
                  break;
                case HOEL_COL_TYPE_BOOL:
                  if (o_strcasecmp(val, "t") == 0) {
                    data = h_new_data_int(1);
                  } else if (o_strcasecmp(val, "f") == 0) {
                    data = h_new_data_int(0);
                  } else {
                    data = h_new_data_null();
                  }
                  break;
                case HOEL_COL_TYPE_DATE:
                case HOEL_COL_TYPE_TEXT:
                default:
                  data = h_new_data_text(val, PQfsize(res, i));
                  break;
              }
            }
            h_res = h_row_add_data(&cur_row, data, j);
            h_clean_data_full(data);
            if (h_res != H_OK) {
              PQclear(res);
              ret = h_res;
            }
          }
          h_res = h_result_add_row(result, cur_row, i);
          if (h_res != H_OK) {
            PQclear(res);
            ret = h_res;
          }
        }
      }
      PQclear(res);
    }
    pthread_mutex_unlock(&(((struct _h_pgsql *)conn->connection)->lock));
  }
  return ret;
}

/**
 * h_execute_query_json_pgsql
 * Execute a query on a pgsql connection, set the returned values in the json results
 * Should not be executed by the user because all parameters are supposed to be correct
 * return H_OK on success
 */
int h_execute_query_json_pgsql(const struct _h_connection * conn, const char * query, json_t ** j_result) {
  PGresult *res;
  int nfields, ntuples, i, j, ret = H_OK;
  json_t * j_data;
  
  if (pthread_mutex_lock(&(((struct _h_pgsql *)conn->connection)->lock))) {
    ret = H_ERROR_QUERY;
  } else {
    if (j_result == NULL) {
      ret = H_ERROR_PARAMS;
    } else {
      *j_result = json_array();
      if (*j_result == NULL) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Hoel - Error allocating memory for *j_result");
        ret = H_ERROR_MEMORY;
      } else {
        res = PQexec(((struct _h_pgsql *)conn->connection)->db_handle, query);
        if (PQresultStatus(res) != PGRES_TUPLES_OK && PQresultStatus(res) != PGRES_COMMAND_OK) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Error executing sql query");
          y_log_message(Y_LOG_LEVEL_DEBUG, "Error message: \"%s\"", PQerrorMessage(((struct _h_pgsql *)conn->connection)->db_handle));
          y_log_message(Y_LOG_LEVEL_DEBUG, "Query: \"%s\"", query);
          ret = H_ERROR_QUERY;
        } else {
          nfields = PQnfields(res);
          ntuples = PQntuples(res);

          for(i = 0; ret == H_OK && i < ntuples; i++) {
            j_data = json_object();
            if (j_data == NULL) {
              y_log_message(Y_LOG_LEVEL_ERROR, "Hoel - Error allocating memory for j_data");
              PQclear(res);
              ret = H_ERROR_MEMORY;
            } else {
              for(j = 0; ret == H_OK && j < nfields; j++) {
                char * val = PQgetvalue(res, i, j);
                if (val == NULL || strlen(val) == 0) {
                  json_object_set_new(j_data, PQfname(res, j), json_null());
                } else {
                  switch (h_get_type_from_oid(conn, PQftype(res, j))) {
                    case HOEL_COL_TYPE_INT:
                      json_object_set_new(j_data, PQfname(res, j), json_integer(strtoll(PQgetvalue(res, i, j), NULL, 10)));
                      break;
                    case HOEL_COL_TYPE_DOUBLE:
                      json_object_set_new(j_data, PQfname(res, j), json_real(strtod(PQgetvalue(res, i, j), NULL)));
                      break;
                    case HOEL_COL_TYPE_BLOB:
                      json_object_set_new(j_data, PQfname(res, j), json_stringn(PQgetvalue(res, i, j), PQfsize(res, i)));
                      break;
                    case HOEL_COL_TYPE_BOOL:
                      if (o_strcasecmp(PQgetvalue(res, i, j), "t") == 0) {
                        json_object_set_new(j_data, PQfname(res, j), json_integer(1));
                      } else if (o_strcasecmp(PQgetvalue(res, i, j), "f") == 0) {
                        json_object_set_new(j_data, PQfname(res, j), json_integer(0));
                      } else {
                        json_object_set_new(j_data, PQfname(res, j), json_null());
                      }
                      break;
                    case HOEL_COL_TYPE_DATE:
                    case HOEL_COL_TYPE_TEXT:
                    default:
                      json_object_set_new(j_data, PQfname(res, j), json_string(PQgetvalue(res, i, j)));
                      break;
                  }
                }
              }
            }
            json_array_append_new(*j_result, j_data);
            j_data = NULL;
          }
        }
        PQclear(res);
      }
    }
    pthread_mutex_unlock(&(((struct _h_pgsql *)conn->connection)->lock));
  }
  
  return ret;
}

/**
 * Return the id of the last inserted value
 * Assuming you use sequences for automatically generated ids
 */
long long int h_last_insert_id_pgsql(const struct _h_connection * conn) {
  PGresult *res;
  long long int int_res = 0;
  char * str_res, * endptr = NULL;
  
  res = PQexec(((struct _h_pgsql *)conn->connection)->db_handle, "SELECT lastval()");
  if (PQresultStatus(res) != PGRES_TUPLES_OK && PQresultStatus(res) != PGRES_COMMAND_OK) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error executing h_last_insert_id");
    y_log_message(Y_LOG_LEVEL_DEBUG, "Error message: \"%s\"", PQerrorMessage(((struct _h_pgsql *)conn->connection)->db_handle));
    return H_ERROR_QUERY;
  }
  
  if (PQnfields(res) && PQntuples(res)) {
    str_res = PQgetvalue(res, 0, 0);
    if (str_res != NULL) {
      int_res = strtol(str_res, &endptr, 10);
      if (*endptr != '\0' || endptr == str_res) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Error h_last_insert_id, returned value can't be converted to numeric");
        int_res = 0;
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error h_last_insert_id, returned value is NULL");
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error h_last_insert_id, returned value has no data available");
  }
  PQclear(res);
  return int_res;
}
#else

/**
 * Dummy functions when Hoel is not built with PostgreSQL
 */
struct _h_connection * h_connect_pgsql(const char * conninfo) {
  UNUSED(conninfo);
  y_log_message(Y_LOG_LEVEL_ERROR, "Hoel was not compiled with PostgreSQL backend");
  return NULL;
}

void h_close_pgsql(struct _h_connection * conn) {
  UNUSED(conn);
  y_log_message(Y_LOG_LEVEL_ERROR, "Hoel was not compiled with PostgreSQL backend");
}

#endif
