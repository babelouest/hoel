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
};

/**
 * h_connect_pgsql
 * Opens a database connection to a PostgreSQL server
 * return pointer to a struct _h_connection * on sucess, NULL on error
 */
struct _h_connection * h_connect_pgsql(char * conninfo) {
  struct _h_connection * conn = NULL;
  struct _h_result result_types;
  int res_types;
  unsigned int row;
  char * cur_type_name, * endptr = NULL;
  Oid cur_type_oid;
  
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
      res_types = h_execute_query_pgsql(conn, "select oid, typname from pg_type", &result_types);
      if (res_types == H_OK) {
        if (result_types.nb_columns == 2) {
          for (row=0; row<result_types.nb_rows; row++) {
            cur_type_oid = strtol(((struct _h_type_text *)result_types.data[row][0].t_data)->value, &endptr, 10);
            cur_type_name = ((struct _h_type_text *)result_types.data[row][1].t_data)->value;
            if (*endptr == '\0' && endptr != ((struct _h_type_text *)result_types.data[row][0].t_data)->value) {
              ((struct _h_pgsql *)conn->connection)->list_type = o_realloc(((struct _h_pgsql *)conn->connection)->list_type, (((struct _h_pgsql *)conn->connection)->nb_type + 1) * sizeof(struct _h_pg_type));
              if (((struct _h_pgsql *)conn->connection)->list_type != NULL) {
                ((struct _h_pgsql *)conn->connection)->list_type[((struct _h_pgsql *)conn->connection)->nb_type].pg_type = cur_type_oid;
                if (o_strcmp(cur_type_name, "bool") == 0) {
                  ((struct _h_pgsql *)conn->connection)->list_type[((struct _h_pgsql *)conn->connection)->nb_type].h_type = HOEL_COL_TYPE_BOOL;
                } else if (o_strncmp(cur_type_name, "int", 3) == 0 || (o_strncmp(cur_type_name+1, "id", 2) == 0 && o_strlen(cur_type_name) == 3)) {
                  ((struct _h_pgsql *)conn->connection)->list_type[((struct _h_pgsql *)conn->connection)->nb_type].h_type = HOEL_COL_TYPE_INT;
                } else if (o_strcmp(cur_type_name, "numeric") == 0 || o_strncmp(cur_type_name, "float", 5) == 0) {
                  ((struct _h_pgsql *)conn->connection)->list_type[((struct _h_pgsql *)conn->connection)->nb_type].h_type = HOEL_COL_TYPE_DOUBLE;
                } else if (o_strcmp(cur_type_name, "date") == 0 || o_strncmp(cur_type_name, "time", 4) == 0) {
                  ((struct _h_pgsql *)conn->connection)->list_type[((struct _h_pgsql *)conn->connection)->nb_type].h_type = HOEL_COL_TYPE_DATE;
                } else if (o_strcmp(cur_type_name, "bytea") == 0) {
                  ((struct _h_pgsql *)conn->connection)->list_type[((struct _h_pgsql *)conn->connection)->nb_type].h_type = HOEL_COL_TYPE_BLOB;
                } else if (o_strcmp(cur_type_name, "bool") == 0) {
                  ((struct _h_pgsql *)conn->connection)->list_type[((struct _h_pgsql *)conn->connection)->nb_type].h_type = HOEL_COL_TYPE_BOOL;
                } else {
                  ((struct _h_pgsql *)conn->connection)->list_type[((struct _h_pgsql *)conn->connection)->nb_type].h_type = HOEL_COL_TYPE_TEXT;
                }
                ((struct _h_pgsql *)conn->connection)->nb_type++;
              } else {
                y_log_message(Y_LOG_LEVEL_ERROR, "Hoel - Error allocating resources for list_type");
              }
            } else {
              y_log_message(Y_LOG_LEVEL_ERROR, "Hoel - Error converting pg_type.oid to integer");
            }
          }
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "Hoel - Error pg_type result");
          PQfinish(((struct _h_pgsql *)conn->connection)->db_handle);
          free(conn->connection);
          free(conn);
          conn = NULL;
        }
        h_clean_result(&result_types);
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "Hoel - Error getting pg_type");
        PQfinish(((struct _h_pgsql *)conn->connection)->db_handle);
        free(conn->connection);
        free(conn);
        conn = NULL;
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
}

/**
 * escape a string
 * returned value must be free'd after use
 */
char * h_escape_string_pgsql(const struct _h_connection * conn, const char * unsafe) {
  return PQescapeLiteral(((struct _h_pgsql *)conn->connection)->db_handle, unsafe, strlen(unsafe));
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
  PGresult *res;
  int nfields, ntuples, i, j, h_res;
  struct _h_data * data, * cur_row = NULL;
  
  res = PQexec(((struct _h_pgsql *)conn->connection)->db_handle, query);
  if (PQresultStatus(res) != PGRES_TUPLES_OK && PQresultStatus(res) != PGRES_COMMAND_OK) {
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
        if (val == NULL) {
          data = h_new_data_null();
        } else {
          switch (h_get_type_from_oid(conn, PQftype(res, j))) {
            case HOEL_COL_TYPE_INT:
              data = h_new_data_int(strtol(PQgetvalue(res, i, j), NULL, 10));
              break;
            case HOEL_COL_TYPE_DOUBLE:
              data = h_new_data_double(strtod(PQgetvalue(res, i, j), NULL));
              break;
            case HOEL_COL_TYPE_BLOB:
              data = h_new_data_blob(PQgetvalue(res, i, j), PQfsize(res, i));
              break;
            case HOEL_COL_TYPE_BOOL:
              if (o_strcasecmp(PQgetvalue(res, i, j), "t") == 0) {
                data = h_new_data_int(1);
              } else if (o_strcasecmp(PQgetvalue(res, i, j), "f") == 0) {
                data = h_new_data_int(0);
              } else {
                data = h_new_data_null();
              }
              break;
            case HOEL_COL_TYPE_DATE:
            case HOEL_COL_TYPE_TEXT:
            default:
              data = h_new_data_text(PQgetvalue(res, i, j), PQfsize(res, i));
              break;
          }
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
  if (PQresultStatus(res) != PGRES_TUPLES_OK && PQresultStatus(res) != PGRES_COMMAND_OK) {
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
        switch (h_get_type_from_oid(conn, PQftype(res, j))) {
          case HOEL_COL_TYPE_INT:
            json_object_set_new(j_data, PQfname(res, j), json_integer(strtol(PQgetvalue(res, i, j), NULL, 10)));
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
    json_array_append_new(*j_result, j_data);
    j_data = NULL;
  }
  PQclear(res);
  return H_OK;
}

/**
 * Return the id of the last inserted value
 * Assuming you use sequences for automatically generated ids
 */
int h_last_insert_id_pgsql(const struct _h_connection * conn) {
  PGresult *res;
  int int_res = 0;
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
struct _h_connection * h_connect_pgsql(char * conninfo) {
  UNUSED(conninfo);
  y_log_message(Y_LOG_LEVEL_ERROR, "Hoel was not compiled with PostgreSQL backend");
  return NULL;
}

void h_close_pgsql(struct _h_connection * conn) {
  UNUSED(conn);
  y_log_message(Y_LOG_LEVEL_ERROR, "Hoel was not compiled with PostgreSQL backend");
}

#endif
