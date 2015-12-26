/**
 * 
 * Hoel database abstraction library
 * 
 * hoel-simple-json.c: hoel simple Json query functions
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

// internal functions declarations
char * h_get_insert_query_from_json_object(const struct _h_connection * conn, json_t * data, const char * table);
char * h_get_where_clause_from_json_object(const struct _h_connection * conn, json_t * where);
char * h_get_set_clause_from_json_object(const struct _h_connection * conn, json_t * set);

/**
 * h_insert
 * Insert data as a json object in table name on the specified conenction
 * data must be an object or an array of objects
 * return H_OK on success
 */
int h_insert(const struct _h_connection * conn, const char * table, json_t * data, char ** generated_query) {
  char * query;
  size_t index;
  json_t * j_row;
  int res;
  
  if (conn != NULL && data != NULL && table != NULL) {
    // Construct query
    switch json_typeof(data) {
      case JSON_OBJECT:
        query = h_get_insert_query_from_json_object(conn, data, table);
        if (query != NULL) {
          if (generated_query != NULL) {
            *generated_query = strdup(query);
          }
          res = h_query_insert(conn, query);
          free(query);
          return res;
        } else {
          return H_ERROR_MEMORY;
        }
        break;
      case JSON_ARRAY:
        json_array_foreach(data, index, j_row) {
          query = h_get_insert_query_from_json_object(conn, j_row, table);
          if (query != NULL) {
            if (generated_query != NULL && index == 0) {
              // Export just the first query
              *generated_query = strdup(query);
            }
            res = h_query_insert(conn, query);
            free(query);
            if (res != H_OK) {
              return H_ERROR_QUERY;
            }
          } else {
            return H_ERROR_MEMORY;
          }
        }
        return H_OK;
        break;
      default:
        return H_ERROR_PARAMS;
        break;
    }
  } else {
    return H_ERROR_PARAMS;
  }
}

/**
 * h_select
 * Execute a select using a table name for the FROM keyword, a json array for the columns, and a json object for the WHERE keyword
 * where must be a where_type json object
 * return H_OK on success
 */
int h_select(const struct _h_connection * conn, const char * table, json_t * cols, json_t * where, json_t ** j_result, char ** generated_query) {
  char * query, * columns = NULL, * where_clause = NULL, * tmp, * dump, * escape;
  size_t index;
  json_t * value;
  int res;

  where_clause = h_get_where_clause_from_json_object(conn, where);
  if (where_clause == NULL) {
    return H_ERROR_PARAMS;
  }
  if (cols == NULL) {
    columns = strdup("*");
  } else if (json_is_array(cols)) {
    json_array_foreach(cols, index, value) {
      if (json_is_string(value)) {
        if (index == 0) {
          dump = json_dumps(value, JSON_ENCODE_ANY);
          escape = h_escape_string(conn, trim_whitespace_and_double_quotes(dump));
          if (escape == NULL) {
            free(where_clause);
            return H_ERROR_MEMORY;
          }
          columns = strdup(escape);
        } else {
          dump = json_dumps(value, JSON_ENCODE_ANY);
          escape = h_escape_string(conn, trim_whitespace_and_double_quotes(dump));
          if (escape == NULL) {
            free(where_clause);
            free(columns);
            free(dump);
            return H_ERROR_MEMORY;
          }
          tmp = h_msprintf("%s, %s", columns, escape);
          if (tmp == NULL) {
            free(where_clause);
            free(columns);
            free(dump);
            free(escape);
            return H_ERROR_MEMORY;
          }
          free(columns);
          columns = tmp;
        }
        free(dump);
        free(escape);
      } else {
        free(where_clause);
        return H_ERROR_PARAMS;
      }
    }
  } else {
    free(where_clause);
    return H_ERROR_PARAMS;
  }
  query = h_msprintf("SELECT %s FROM %s WHERE %s", columns, table, where_clause);
  if (query == NULL) {
    free(columns);
    free(where_clause);
    return H_ERROR_MEMORY;
  } else {
    if (generated_query != NULL) {
      *generated_query = strdup(query);
    }
    res = h_query_select_json(conn, query, j_result);
    free(columns);
    free(where_clause);
    free(query);
    return res;
  }
}

/**
 * h_update
 * Update data using a json object and a table name and a where clause
 * data must be an object, where must be a where_type json object
 * return H_OK on success
 */
int h_update(const struct _h_connection * conn, const char * table, json_t * set, json_t * where, char ** generated_query) {
  char * set_clause, * where_clause, * query;
  int res;
  
  set_clause = h_get_set_clause_from_json_object(conn, set);
  where_clause = h_get_where_clause_from_json_object(conn, where);
  
  if (set_clause == NULL || where_clause == NULL) {
    return H_ERROR_PARAMS;
  }
  query = h_msprintf("UPDATE %s SET %s WHERE %s", table, set_clause, where_clause);
  free(set_clause);
  free(where_clause);
  if (query == NULL) {
    return H_ERROR_MEMORY;
  }
  if (generated_query != NULL) {
    *generated_query = strdup(query);
  }
  res = h_query_update(conn, query);
  free(query);
  return res;
}

/**
 * h_delete
 * Delete data using a table name and a where clause
 * where must be a where_type json object
 * return H_OK on success
 */
int h_delete(const struct _h_connection * conn, const char * table, json_t * where, char ** generated_query) {
  char * where_clause, * query;
  int res;
  
  where_clause = h_get_where_clause_from_json_object(conn, where);
  
  if (where_clause == NULL) {
    return H_ERROR_PARAMS;
  }
  query = h_msprintf("DELETE FROM %s WHERE %s", table, where_clause);
  free(where_clause);
  if (query == NULL) {
    return H_ERROR_MEMORY;
  }
  if (generated_query != NULL) {
    *generated_query = strdup(query);
  }
  res = h_query_delete(conn, query);
  free(query);
  return res;
}

/**
 * Builds an insert query from a json object and a table name
 * Returned value must be free'd after use
 */
char * h_get_insert_query_from_json_object(const struct _h_connection * conn, json_t * data, const char * table) {
  char * key, * insert_cols = NULL, * insert_data = NULL, * escape, * dump, * to_return;
  int i = 0;
  json_t * value;
  
  json_object_foreach(data, key, value) {
    if (i == 0) {
      dump = json_dumps(value, JSON_ENCODE_ANY);
      escape = h_escape_string(conn, trim_whitespace_and_double_quotes(dump));
      free(dump);
      if (escape == NULL) {
        return NULL;
      }
      insert_cols = malloc(strlen(key)+1);
      if (insert_cols == NULL) {
        free(escape);
        return NULL;
      }
      insert_data = malloc(strlen(escape)+3);
      if (insert_data == NULL) {
        free(escape);
        free(insert_cols);
        return NULL;
      }
      strcpy(insert_cols, key);
      strcpy(insert_data, "'");
      strcat(insert_data, escape);
      strcat(insert_data, "'");
    } else {
      dump = json_dumps(value, JSON_ENCODE_ANY);
      escape = h_escape_string(conn, trim_whitespace_and_double_quotes(dump));
      free(dump);
      if (escape == NULL) {
        free(insert_cols);
        free(insert_data);
        return NULL;
      }
      insert_cols = realloc(insert_cols, strlen(insert_cols)+strlen(key)+2);
      if (insert_cols == NULL) {
        free(escape);
        free(insert_data);
        return NULL;
      }
      insert_data = realloc(insert_data, strlen(insert_data)+strlen(escape)+4);
      if (insert_data == NULL) {
        free(escape);
        free(insert_cols);
        return NULL;
      }
      strcat(insert_cols, ",");
      strcat(insert_cols, key);
      strcat(insert_data, ",");
      strcat(insert_data, "'");
      strcat(insert_data, escape);
      strcat(insert_data, "'");
    }
    free(escape);
    i++;
  }
  to_return = h_msprintf("INSERT INTO %s (%s) VALUES (%s)", table, insert_cols, insert_data);
  free(insert_cols);
  free(insert_data);
  return to_return;
}

/**
 * Generates a where clause based on a json object
 * the where object is a simple object like
 * {
 *   col1: "value1",
 *   col2: "value2"
 * }
 * the output is a WHERE query will use only '=' and 'AND' keywords
 * col1='value1' AND col2='value2'
 * return a char * containing the WHERE clause, NULL on error
 * the returned value must be free'd after use
 */
char * h_get_where_clause_from_json_object(const struct _h_connection * conn, json_t * where) {
  const char * key;
  json_t * value, * ope, * val;
  char * where_clause = NULL, * dump = NULL, * escape = NULL, * tmp, * clause;
  int i = 0;
  
  if (conn == NULL) {
    return NULL;
  } else if (where == NULL || (json_is_object(where) && json_object_size(where) == 0)) {
    return strdup("1=1");
  } else {
    json_object_foreach(where, key, value) {
      if (!json_is_string(value) && !json_is_real(value) && !json_is_integer(value) && !json_is_object(value)) {
        free(where_clause);
        return NULL;
      } else {
        if (json_is_object(value)) {
          ope = json_object_get(value, "operator");
          val = json_object_get(value, "value");
          if (ope == NULL || !json_is_string(ope) || val == NULL || (!json_is_string(val) && !json_is_real(val) && !json_is_integer(val))) {
            free(where_clause);
            return NULL;
          } else {
            dump = json_dumps(val, JSON_ENCODE_ANY);
            escape = h_escape_string(conn, trim_whitespace_and_double_quotes(dump));
            clause = h_msprintf("%s %s '%s'", key, json_string_value(ope), json_string_value(val));
            free(dump);
            free(escape);
          }
        } else {
          dump = json_dumps(value, JSON_ENCODE_ANY);
          escape = h_escape_string(conn, trim_whitespace_and_double_quotes(dump));
          clause = h_msprintf("%s = '%s'", key, escape);
          free(dump);
          free(escape);
        }
        if (i == 0) {
          where_clause = h_msprintf("%s", clause);
          if (where_clause == NULL) {
            free(clause);
            return NULL;
          }
          free(clause);
          i = 1;
        } else {
          tmp = h_msprintf("%s AND %s", where_clause, clause);
          free(where_clause);
          if (tmp == NULL) {
            free(clause);
            return NULL;
          }
          free(clause);
          where_clause = tmp;
        }
      }
    }
    return where_clause;
  }
}

/**
 * Generates a set clause based on a json object
 * the where object is a simple object like
 * {
 *   col1: "value1",
 *   col2: "value2"
 * }
 * the output is a WHERE query will use only '=' and 'AND' keywords
 * col1='value1', col2='value2'
 * return a char * containing the WHERE clause, NULL on error
 * the returned value must be free'd after use
 */
char * h_get_set_clause_from_json_object(const struct _h_connection * conn, json_t * set) {
  const char * key;
  json_t * value;
  char * where_clause = NULL, * dump = NULL, * escape = NULL, * tmp;
  int i = 0;
  
  if (conn == NULL || set == NULL || !json_is_object(set)) {
    return NULL;
  } else {
    json_object_foreach(set, key, value) {
      if (!json_is_string(value) && !json_is_real(value) && !json_is_integer(value)) {
        free(where_clause);
        return NULL;
      } else {
        dump = json_dumps(value, JSON_ENCODE_ANY);
        escape = h_escape_string(conn, trim_whitespace_and_double_quotes(dump));
        if (i == 0) {
          where_clause = h_msprintf("%s='%s'", key, escape);
          if (where_clause == NULL) {
            return NULL;
          }
          i = 1;
        } else {
          tmp = h_msprintf("%s, %s='%s'", where_clause, key, escape);
          free(where_clause);
          if (tmp == NULL) {
            return NULL;
          }
          where_clause = tmp;
        }
        free(dump);
        free(escape);
      }
    }
    return where_clause;
  }
}
