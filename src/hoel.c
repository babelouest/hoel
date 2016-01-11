/**
 * 
 * Hoel database abstraction library
 * 
 * hoel.c: main functions
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
#include <ctype.h>

// Get rid of noisy warning
char * strcasestr (const char *haystack, const char *needle);

/**
 * Close a database connection
 * return H_OK on success
 */
int h_close_db(struct _h_connection * conn) {
  if (conn != NULL && conn->connection != NULL) {
    if (0) {
      // Not happening
#ifdef _HOEL_SQLITE
    } else if (conn->type == HOEL_DB_TYPE_SQLITE) {
      h_close_sqlite(conn);
      return H_OK;
#endif
#ifdef _HOEL_MARIADB
    } else if (conn->type == HOEL_DB_TYPE_MARIADB) {
      h_close_mariadb(conn);
      return H_OK;
#endif
#ifdef _HOEL_PGSQL
    } else if (conn->type == HOEL_DB_TYPE_PGSQL) {
      h_close_pgsql(conn);
      return H_OK;
#endif
    } else {
      return H_ERROR_PARAMS;
    }
  } else {
    return H_ERROR_PARAMS;
  }
}

/**
 * h_escape_string
 * Escapes a string
 * returned value must be free'd after use
 */
char * h_escape_string(const struct _h_connection * conn, const char * unsafe) {
  if (conn != NULL && conn->connection != NULL && unsafe != NULL) {
    if (0) {
      // Not happening
#ifdef _HOEL_SQLITE
    } else if (conn->type == HOEL_DB_TYPE_SQLITE) {
      return h_escape_string_sqlite(conn, unsafe);
#endif
#ifdef _HOEL_MARIADB
    } else if (conn->type == HOEL_DB_TYPE_MARIADB) {
      return h_escape_string_mariadb(conn, unsafe);
#endif
#ifdef _HOEL_PGSQL
    } else if (conn->type == HOEL_DB_TYPE_PGSQL) {
      return h_escape_string_pgsql(conn, unsafe);
#endif
    } else {
      return NULL;
    }
  } else {
    return NULL;
  }
}

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
int h_execute_query(const struct _h_connection * conn, const char * query, struct _h_result * result, int options) {
  if (conn != NULL && conn->connection != NULL && query != NULL) {
    if (0) {
      // Not happening
#ifdef _HOEL_SQLITE
    } else if (conn->type == HOEL_DB_TYPE_SQLITE) {
      if (options & H_OPTION_EXEC) {
        return h_exec_query_sqlite(conn, query);
      } else {
        return h_select_query_sqlite(conn, query, result);
      }
#endif
#ifdef _HOEL_MARIADB
    } else if (conn->type == HOEL_DB_TYPE_MARIADB) {
      return h_execute_query_mariadb(conn, query, result);
#endif
#ifdef _HOEL_PGSQL
    } else if (conn->type == HOEL_DB_TYPE_PGSQL) {
      return h_execute_query_pgsql(conn, query, result);
#endif
    } else {
      return H_ERROR_PARAMS;
    }
  } else {
    return H_ERROR_PARAMS;
  }
}

/**
 * h_execute_query_json
 * Execute a query, set the returned values in the json result
 * return H_OK on success
 */
int h_execute_query_json(const struct _h_connection * conn, const char * query, json_t ** j_result) {
  if (conn != NULL && conn->connection != NULL && query != NULL) {
    if (0) {
      // Not happening
#ifdef _HOEL_SQLITE
    } else if (conn->type == HOEL_DB_TYPE_SQLITE) {
      return h_execute_query_json_sqlite(conn, query, j_result);
#endif
#ifdef _HOEL_MARIADB
    } else if (conn->type == HOEL_DB_TYPE_MARIADB) {
      return h_execute_query_json_mariadb(conn, query, j_result);
#endif
#ifdef _HOEL_PGSQL
    } else if (conn->type == HOEL_DB_TYPE_PGSQL) {
      return h_execute_query_json_pgsql(conn, query, j_result);
#endif
    } else {
      return H_ERROR_PARAMS;
    }
  } else {
    return H_ERROR_PARAMS;
  }
}

/**
 * Add a new struct _h_data * to an array of struct _h_data *, which already has cols columns
 * return H_OK on success
 */
int h_row_add_data(struct _h_data ** row, struct _h_data * data, int cols) {
  struct _h_data * tmp = realloc(*row, (cols+1)*sizeof(struct _h_data));
  * row = tmp;
  if (tmp == NULL) {
    return H_ERROR_MEMORY;
  } else {
    switch (data->type) {
      case HOEL_COL_TYPE_INT:
        tmp[cols].type = HOEL_COL_TYPE_INT;
        tmp[cols].t_data = malloc(sizeof(struct _h_type_int));
        if (tmp[cols].t_data == NULL) {
          return H_ERROR_MEMORY;
        } else {
          ((struct _h_type_int *)tmp[cols].t_data)->value = ((struct _h_type_int *)data->t_data)->value;
          return H_OK;
        }
        break;
      case HOEL_COL_TYPE_DOUBLE:
        tmp[cols].type = HOEL_COL_TYPE_DOUBLE;
        tmp[cols].t_data = malloc(sizeof(struct _h_type_double));
        if (tmp[cols].t_data == NULL) {
          return H_ERROR_MEMORY;
        } else {
          ((struct _h_type_double *)tmp[cols].t_data)->value = ((struct _h_type_double *)data->t_data)->value;
          return H_OK;
        }
        break;
      case HOEL_COL_TYPE_TEXT:
        tmp[cols].type = HOEL_COL_TYPE_TEXT;
        tmp[cols].t_data = malloc(sizeof(struct _h_type_text));
        if (tmp[cols].t_data == NULL) {
          return H_ERROR_MEMORY;
        } else {
          ((struct _h_type_text *)tmp[cols].t_data)->value = malloc(strlen(((struct _h_type_text *)data->t_data)->value)+sizeof(char));
          if (((struct _h_type_text *)tmp[cols].t_data)->value == NULL) {
            free(tmp[cols].t_data);
            return H_ERROR_MEMORY;
          }
          strncpy(((struct _h_type_text *)tmp[cols].t_data)->value, ((struct _h_type_text *)data->t_data)->value, (strlen(((struct _h_type_text *)data->t_data)->value)+1));
          return H_OK;
        }
        break;
      case HOEL_COL_TYPE_BLOB:
        tmp[cols].type = HOEL_COL_TYPE_BLOB;
        tmp[cols].t_data = malloc(sizeof(struct _h_type_blob));
        if (tmp[cols].t_data == NULL) {
          return H_ERROR_MEMORY;
        } else {
          ((struct _h_type_blob *)tmp[cols].t_data)->length = ((struct _h_type_blob *)data->t_data)->length;
          ((struct _h_type_blob *)tmp[cols].t_data)->value = malloc(((struct _h_type_blob *)data->t_data)->length);
          if (((struct _h_type_blob *)tmp[cols].t_data)->value == NULL) {
            free(tmp[cols].t_data);
            return H_ERROR_MEMORY;
          }
          memcpy(((struct _h_type_blob *)tmp[cols].t_data)->value, ((struct _h_type_blob *)data->t_data)->value, ((struct _h_type_blob *)data->t_data)->length);
          return H_OK;
        }
        break;
      case HOEL_COL_TYPE_DATE:
        tmp[cols].type = HOEL_COL_TYPE_DATE;
        tmp[cols].t_data = malloc(sizeof(struct _h_type_datetime));
        if (tmp[cols].t_data == NULL) {
          return H_ERROR_MEMORY;
        } else {
          ((struct _h_type_datetime *)tmp[cols].t_data)->value = ((struct _h_type_datetime *)data->t_data)->value;
          return H_OK;
        }
        break;
      case HOEL_COL_TYPE_NULL:
        tmp[cols].type = HOEL_COL_TYPE_NULL;
        tmp[cols].t_data = NULL;
        break;
      default:
        return H_ERROR_PARAMS;
        break;
    }
    return H_OK;
  }
}

/**
 * Add a new row of struct _h_data * in a struct _h_result *
 * return H_OK on success
 */
int h_result_add_row(struct _h_result * result, struct _h_data * row, int rows) {
  result->data = realloc(result->data, (rows+1)*sizeof(struct _h_data *));
  if (result->data == NULL) {
    return H_ERROR_MEMORY;
  } else {
    result->data[rows] = row;
    result->nb_rows++;
    return H_OK;
  }
}

/**
 * h_query_insert
 * Execute an insert query
 * return H_OK on success
 */
int h_query_insert(const struct _h_connection * conn, const char * query) {
  if (conn != NULL && conn->connection != NULL && query != NULL && strcasestr(query, "insert") != NULL) {
    return h_execute_query(conn, query, NULL, H_OPTION_EXEC);
  } else {
    return H_ERROR_PARAMS;
  }
}

/**
 * h_query_last_insert_id
 * return the id of the last inserted value
 * return H_OK on success
 */
struct _h_data * h_query_last_insert_id(const struct _h_connection * conn) {
  struct _h_data * data = NULL;
  if (conn != NULL && conn->connection != NULL) {
    if (0) {
      // Not happening
#ifdef _HOEL_SQLITE
    } else if (conn->type == HOEL_DB_TYPE_SQLITE) {
      int last_id = h_last_insert_id_sqlite(conn);
      if (last_id > 0) {
        data = h_new_data_int(last_id);
      } else {
        data = h_new_data_null();
      }
#endif
#ifdef _HOEL_MARIADB
    } else if (conn->type == HOEL_DB_TYPE_MARIADB) {
      int last_id = h_last_insert_id_mariadb(conn);
      if (last_id > 0) {
        data = h_new_data_int(last_id);
      } else {
        data = h_new_data_null();
      }
#endif
#ifdef _HOEL_PGSQL
    } else if (conn->type == HOEL_DB_TYPE_PGSQL) {
      // TODO
      // Not possible ?
      y_log_message(Y_LOG_LEVEL_ERROR, "Error feature not supported");
      data = h_new_data_null();
#endif
    } else {
      data = h_new_data_null();
    }
  }
  return data;
}

/**
 * h_query_update
 * Execute an update query
 * return H_OK on success
 */
int h_query_update(const struct _h_connection * conn, const char * query) {
  if (conn != NULL && conn->connection != NULL && query != NULL && strcasestr(query, "update") != NULL) {
    return h_execute_query(conn, query, NULL, H_OPTION_EXEC);
  } else {
    return H_ERROR_PARAMS;
  }
}

/**
 * h_query_delete
 * Execute an delete query
 * return H_OK on success
 */
int h_query_delete(const struct _h_connection * conn, const char * query) {
  if (conn != NULL && conn->connection != NULL && query != NULL && strcasestr(query, "delete") != NULL) {
    return h_execute_query(conn, query, NULL, H_OPTION_EXEC);
  } else {
    return H_ERROR_PARAMS;
  }
}

/**
 * h_query_select
 * Execute a select query, set the result structure with the returned values
 * return H_OK on success
 */
int h_query_select(const struct _h_connection * conn, const char * query, struct _h_result * result) {
  if (conn != NULL && conn->connection != NULL && query != NULL && strcasestr(query, "select") != NULL) {
    return h_execute_query(conn, query, result, H_OPTION_SELECT);
  } else {
    return H_ERROR_PARAMS;
  }
}

/**
 * h_query_select_json
 * Execute a select query, set the returned values in the json results
 * return H_OK on success
 */
int h_query_select_json(const struct _h_connection * conn, const char * query, json_t ** j_result) {
  if (conn != NULL && conn->connection != NULL && query != NULL && strcasestr(query, "select") != NULL) {
    return h_execute_query_json(conn, query, j_result);
  } else {
    return H_ERROR_PARAMS;
  }
}

/**
 * trim_whitespace_and_double_quotes
 * Return the string without its beginning and ending whitespaces or double quotes
 */
char * trim_whitespace_and_double_quotes(char *str) {
  char *end;

  // Trim leading space
  while(isspace(*str) || *str == '"') str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && (isspace(*end) || *end == '"')) end--;

  // Write new null terminator
  *(end+1) = 0;

  return str;
}

/**
 * h_clean_result
 * Free all the memory allocated by the struct _h_result
 */
int h_clean_result(struct _h_result * result) {
  int col, row;
  if (result != NULL) {
    for (row=0; row<result->nb_rows; row++) {
      for (col=0; col<result->nb_columns; col++) {
        if (h_clean_data(&result->data[row][col]) != H_OK) {
          return H_ERROR_MEMORY;
        }
      }
      free(result->data[row]);
    }
    free(result->data);
    return H_OK;
  } else {
    return H_ERROR_PARAMS;
  }
}

/**
 * h_clean_data
 * Free memory allocated by the struct _h_data
 * return H_OK on success
 */
int h_clean_data(struct _h_data * data) {
  if (data != NULL) {
    if (data->type == HOEL_COL_TYPE_TEXT) {
      free(((struct _h_type_text *)data->t_data)->value);
    } else if (data->type == HOEL_COL_TYPE_BLOB) {
      free(((struct _h_type_blob *)data->t_data)->value);
    }
    if (data->t_data != NULL) {
      free(data->t_data);
    }
    return H_OK;
  } else {
    return H_ERROR_PARAMS;
  }
}

/**
 * h_clean_data_full
 * Free memory allocated by the struct _h_data and the struct _h_data pointer
 * return H_OK on success
 */
int h_clean_data_full(struct _h_data * data) {
  if (data != NULL) {
    h_clean_data(data);
    free(data);
    return H_OK;
  } else {
    return H_ERROR_PARAMS;
  }
}

/**
 * Allocate memory for a new struct _h_data * containing an int
 * return pointer to the new structure
 * return NULL on error
 */
struct _h_data * h_new_data_int(const int value) {
  struct _h_data * data = malloc(sizeof(struct _h_data));
  if (data != NULL) {
    data->t_data = malloc(sizeof(struct _h_type_int));
    if (data->t_data == NULL) {
      free(data);
      return NULL;
    }
    data->type = HOEL_COL_TYPE_INT;
    ((struct _h_type_int *)data->t_data)->value = value;
  }
  return data;
}

/**
 * Allocate memory for a new struct _h_data * containing a double
 * return pointer to the new structure
 * return NULL on error
 */
struct _h_data * h_new_data_double(const double value) {
  struct _h_data * data = malloc(sizeof(struct _h_data));
  if (data != NULL) {
    data->t_data = malloc(sizeof(struct _h_type_double));
    if (data->t_data == NULL) {
      free(data);
      return NULL;
    }
    data->type = HOEL_COL_TYPE_DOUBLE;
    ((struct _h_type_double *)data->t_data)->value = value;
  }
  return data;
}

/**
 * Allocate memory for a new struct _h_data * containing a text
 * return pointer to the new structure
 * return NULL on error
 */
struct _h_data * h_new_data_text(const char * value) {
  struct _h_data * data = malloc(sizeof(struct _h_data));
  if (data != NULL) {
    data->t_data = malloc(sizeof(struct _h_type_text));
    if (data->t_data == NULL) {
      free(data);
      return NULL;
    }
    data->type = HOEL_COL_TYPE_TEXT;
    ((struct _h_type_text *)data->t_data)->value = malloc(strlen(value)+sizeof(char));
    if (((struct _h_type_text *)data->t_data)->value == NULL) {
      free(data);
      return NULL;
    } else {
      strncpy(((struct _h_type_text *)data->t_data)->value, value, (strlen(value)+sizeof(char)));
    }
  }
  return data;
}

/**
 * Allocate memory for a new struct _h_data * containing a blob
 * return pointer to the new structure
 * return NULL on error
 */
struct _h_data * h_new_data_blob(const void * value, const size_t length) {
  struct _h_data * data = malloc(sizeof(struct _h_data));
  if (data != NULL) {
    data->t_data = malloc(sizeof(struct _h_type_blob));
    if (data->t_data == NULL) {
      free(data);
      return NULL;
    }
    data->type = HOEL_COL_TYPE_BLOB;
    ((struct _h_type_blob *)data->t_data)->length = length;
    ((struct _h_type_blob *)data->t_data)->value = malloc(length);
    if (((struct _h_type_blob *)data->t_data)->value == NULL) {
      free(data);
      return NULL;
    } else {
      memcpy(((struct _h_type_blob *)data->t_data)->value, value, length);
    }
  }
  return data;
}

/**
 * Allocate memory for a new struct _h_data * containing a null value
 * return pointer to the new structure
 * return NULL on error
 */
struct _h_data * h_new_data_null() {
  struct _h_data * data = malloc(sizeof(struct _h_data));
  if (data != NULL) {
    data->type = HOEL_COL_TYPE_NULL;
    data->t_data = NULL;
  }
  return data;
}

/**
 * Allocate memory for a new struct _h_data * containing a date time structure
 * return pointer to the new structure
 * return NULL on error
 */
struct _h_data * h_new_data_datetime(const struct tm * datetime) {
  struct _h_data * data = NULL;
  if (datetime != NULL) {
    data = malloc(sizeof(struct _h_data));
    if (data != NULL) {
      data->type = HOEL_COL_TYPE_DATE;
      data->t_data = malloc(sizeof(struct _h_type_datetime));
      
      if (data->t_data == NULL) {
        free(data);
        return NULL;
      }
      ((struct _h_type_datetime *)data->t_data)->value = * datetime;
    }
  }
  return data;
}

/**
 * h_clean_connection
 * free memory allocated by the struct _h_connection
 * return H_OK on success
 */
int h_clean_connection(struct _h_connection * conn) {
  if (conn != NULL) {
    free(conn->connection);
    free(conn);
    return H_OK;
  } else {
    return H_ERROR_PARAMS;
  }
}
