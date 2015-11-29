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

#include "hoel.h"

// MariaDB library Includes
#include <my_global.h>
#include <mysql.h>

// SQLite library includes
#include <sqlite3.h>

// PostgreSQL library includes
#include <libpq-fe.h>

/**
 * SQLite handle
 */
struct _h_sqlite {
  sqlite3 * db_handle;
};

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
 * Postgre SQL handle
 */
struct _h_pgsql {
  char * conninfo;
  PGconn * db_handle;
};

/**
 * h_connect_sqlite
 * Opens a database connection to a sqlite3 db file
 * return H_OK on success
 */
struct _h_connection * h_connect_sqlite(const char * db_path) {
  struct _h_connection * conn = NULL;
  if (db_path != NULL) {
    conn = malloc(sizeof(struct _h_connection));
    if (conn == NULL) {
      return NULL;
    }
    
    conn->type = HOEL_DB_TYPE_SQLITE;
    conn->enabled = 1;
    conn->connection = malloc(sizeof(struct _h_sqlite));
    if (conn->connection == NULL) {
      free(conn);
      return NULL;
    }
    if (sqlite3_open_v2(db_path, &((struct _h_sqlite *)conn->connection)->db_handle, SQLITE_OPEN_READWRITE, NULL) != SQLITE_OK) {
      sqlite3_close_v2(((struct _h_sqlite *)conn->connection)->db_handle);
      conn->enabled = 0;
      return NULL;
    } else {
      return conn;
    }
  }
  return conn;
}

/**
 * h_connect_mariadb
 * Opens a database connection to a mariadb server
 * return H_OK on success
 */
struct _h_connection * h_connect_mariadb(char * host, char * user, char * passwd, char * db, unsigned int port, char * unix_socket) {
  struct _h_connection * conn = NULL;
  if (host != NULL && db != NULL) {
    conn = malloc(sizeof(struct _h_connection));
    if (conn == NULL) {
      return NULL;
    }
    
    conn->type = HOEL_DB_TYPE_MARIADB;
    conn->enabled = 1;
    conn->connection = malloc(sizeof(struct _h_mariadb));
    if (conn->connection == NULL) {
      free(conn);
      return NULL;
    }
    ((struct _h_mariadb *)conn->connection)->db_handle = mysql_init(NULL);
    if (((struct _h_mariadb *)conn->connection)->db_handle == NULL) {
      return NULL;
    }
    if (mysql_real_connect(((struct _h_mariadb *)conn->connection)->db_handle,
                           host, user, passwd, db, port, unix_socket, CLIENT_COMPRESS) == NULL) {
      mysql_close(((struct _h_mariadb *)conn->connection)->db_handle);
      return NULL;
    } else {
      return conn;
    }
  }
  return conn;
}

/**
 * h_connect_pgsql
 * Opens a database connection to a PostgreSQL server
 * return H_OK on success
 */
struct _h_connection * h_connect_pgsql(char * conninfo) {
  // TODO get oids and types
  struct _h_connection * conn = NULL;
  if (conninfo != NULL) {
    conn = malloc(sizeof(struct _h_connection));
    if (conn == NULL) {
      return NULL;
    }
    
    conn->type = HOEL_DB_TYPE_PGSQL;
    conn->enabled = 1;
    conn->connection = malloc(sizeof(struct _h_pgsql));
    if (conn->connection == NULL) {
      free(conn);
      return NULL;
    }
    ((struct _h_pgsql *)conn->connection)->db_handle = PQconnectdb(conninfo);
    
    if (PQstatus(((struct _h_pgsql *)conn->connection)->db_handle) != CONNECTION_OK) {
      PQfinish(((struct _h_pgsql *)conn->connection)->db_handle);
      free(conn->connection);
      free(conn);
      return NULL;
    }
  }
  return conn;
}

/**
 * Close a database connection
 * return H_OK on success
 */
int h_close_db(struct _h_connection * conn) {
  if (conn != NULL && conn->connection != NULL) {
    if (conn->enabled) {
      if (conn->type == HOEL_DB_TYPE_SQLITE) {
        sqlite3_close_v2(((struct _h_sqlite *)conn->connection)->db_handle);
        return H_OK;
      } else if (conn->type == HOEL_DB_TYPE_MARIADB) {
        mysql_close(((struct _h_mariadb *)conn->connection)->db_handle);
        mysql_library_end();
        return H_OK;
      } else if (conn->type == HOEL_DB_TYPE_PGSQL) {
        PQfinish(((struct _h_pgsql *)conn->connection)->db_handle);
        return H_OK;
      } else {
        return H_ERROR_PARAMS;
      }
    } else {
      return H_ERROR_DISABLED;
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
  char * escaped = NULL;
  if (conn != NULL && conn->connection != NULL && unsafe != NULL) {
    if (conn->enabled) {
      if (conn->type == HOEL_DB_TYPE_SQLITE) {
        char * tmp = sqlite3_mprintf("%q", unsafe);
        if (tmp == NULL) {
          return NULL;
        }
        escaped = strdup(tmp);
        sqlite3_free(tmp);
        return escaped;
      } else if (conn->type == HOEL_DB_TYPE_MARIADB) {
        escaped = malloc(2 * strlen(unsafe) + sizeof(char));
        if (escaped == NULL) {
          return NULL;
        }
        mysql_real_escape_string(((struct _h_mariadb *)conn->connection)->db_handle, escaped, unsafe, strlen(unsafe));
        return escaped;
      } else if (conn->type == HOEL_DB_TYPE_PGSQL) {
        return PQescapeLiteral(((struct _h_pgsql *)conn->connection)->db_handle, unsafe, strlen(unsafe));
      } else {
        return NULL;
      }
    } else {
      return NULL;
    }
  } else {
    return NULL;
  }
}

/**
 * h_execute_query
 * Execute a query, set the result structure with the returned values
 * if result is NULL, the query is executed but no value will be returned
 * return H_OK on success
 */
int h_execute_query(const struct _h_connection * conn, const char * query, struct _h_result * result) {
  if (conn != NULL && conn->connection != NULL && query != NULL && result != NULL) {
    if (conn->enabled) {
      if (conn->type == HOEL_DB_TYPE_SQLITE) {
        return h_execute_query_sqlite(conn, query, result);
      } else if (conn->type == HOEL_DB_TYPE_MARIADB) {
        return h_execute_query_mariadb(conn, query, result);
      } else if (conn->type == HOEL_DB_TYPE_PGSQL) {
        return h_execute_query_pgsql(conn, query, result);
      } else {
        return H_ERROR_PARAMS;
      }
    } else {
      return H_ERROR_DISABLED;
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
 * h_execute_query_sqlite
 * Execute a query on a sqlite connection, set the result structure with the returned values
 * Should not be executed by the user because all parameters are supposed to be not null
 * if result is NULL, the query is executed but no value will be returned
 * return H_OK on success
 */
int h_execute_query_sqlite(const struct _h_connection * conn, const char * query, struct _h_result * result) {
  sqlite3_stmt *stmt;
  int sql_result, row_result, nb_columns, col, row, res;
  struct _h_data * data = NULL, * cur_row = NULL;
  
  sql_result = sqlite3_prepare_v2(((struct _h_sqlite *)conn->connection)->db_handle, query, strlen(query)+1, &stmt, NULL);
  
  if (sql_result == SQLITE_OK) {
    nb_columns = sqlite3_column_count(stmt);
    row_result = sqlite3_step(stmt);
    row = 0;
    if (result != NULL) {
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
      sqlite3_finalize(stmt);
      return H_OK;
    } else {
      sqlite3_finalize(stmt);
      return H_OK;
    }
  } else {
    sqlite3_finalize(stmt);
    return H_ERROR_QUERY;
  }
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
    return H_ERROR_QUERY;
  }
  
  if (h_result != NULL) {
    result = mysql_store_result(((struct _h_mariadb *)conn->connection)->db_handle);
    
    if (result == NULL) {
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
 * h_query_insert
 * Execute an insert query
 * return H_OK on success
 */
int h_query_insert(const struct _h_connection * conn, const char * query) {
  if (conn != NULL && conn->connection != NULL) {
    if (conn->enabled) {
      if (conn->type == HOEL_DB_TYPE_SQLITE) {
        // TODO
        return H_OK;
      } else if (conn->type == HOEL_DB_TYPE_MARIADB) {
        // TODO
        return H_OK;
      } else if (conn->type == HOEL_DB_TYPE_PGSQL) {
        // TODO
        return H_OK;
      } else {
        return H_ERROR_PARAMS;
      }
    } else {
      return H_ERROR_DISABLED;
    }
  } else {
    return H_ERROR_PARAMS;
  }
}

/**
 * h_last_insert_id
 * return the id of the last inserted value
 * return H_OK on success
 */
struct _h_data * h_last_insert_id(const struct _h_connection * conn, struct _h_result * result) {
  if (conn != NULL && conn->connection != NULL) {
    if (conn->enabled) {
      if (conn->type == HOEL_DB_TYPE_SQLITE) {
        // TODO
        return NULL;
      } else if (conn->type == HOEL_DB_TYPE_MARIADB) {
        // TODO
        return NULL;
      } else if (conn->type == HOEL_DB_TYPE_PGSQL) {
        // TODO
        return NULL;
      } else {
        return NULL;
      }
    } else {
      return NULL;
    }
  } else {
    return NULL;
  }
}

/**
 * h_query_update
 * Execute an update query
 * return H_OK on success
 */
int h_query_update(const struct _h_connection * conn, const char * query) {
  if (conn != NULL && conn->connection != NULL) {
    if (conn->enabled) {
      if (conn->type == HOEL_DB_TYPE_SQLITE) {
        // TODO
        return H_OK;
      } else if (conn->type == HOEL_DB_TYPE_MARIADB) {
        // TODO
        return H_OK;
      } else if (conn->type == HOEL_DB_TYPE_PGSQL) {
        // TODO
        return H_OK;
      } else {
        return H_ERROR_PARAMS;
      }
    } else {
      return H_ERROR_DISABLED;
    }
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
  if (conn != NULL && conn->connection != NULL) {
    if (conn->enabled) {
      if (conn->type == HOEL_DB_TYPE_SQLITE) {
        // TODO
        return H_OK;
      } else if (conn->type == HOEL_DB_TYPE_MARIADB) {
        // TODO
        return H_OK;
      } else if (conn->type == HOEL_DB_TYPE_PGSQL) {
        // TODO
        return H_OK;
      } else {
        return H_ERROR_PARAMS;
      }
    } else {
      return H_ERROR_DISABLED;
    }
  } else {
    return H_ERROR_PARAMS;
  }
}

/**
 * h_execute_query
 * Execute a select query, set the result structure with the returned values
 * return H_OK on success
 */
int h_query_select(const struct _h_connection * conn, const char * query, struct _h_result * result) {
  if (conn != NULL && conn->connection != NULL) {
    if (conn->enabled) {
      if (conn->type == HOEL_DB_TYPE_SQLITE) {
        // TODO
        return H_OK;
      } else if (conn->type == HOEL_DB_TYPE_MARIADB) {
        // TODO
        return H_OK;
      } else if (conn->type == HOEL_DB_TYPE_PGSQL) {
        // TODO
        return H_OK;
      } else {
        return H_ERROR_PARAMS;
      }
    } else {
      return H_ERROR_DISABLED;
    }
  } else {
    return H_ERROR_PARAMS;
  }
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
