/**
 * 
 * Hoel database abstraction library
 * 
 * hoel-simple-json.c: hoel simple Json query functions
 * 
 * Copyright 2015-2016 Nicolas Mora <mail@babelouest.org>
 *
 * This program is o_free software; you can redistribute it and/or
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

#include <string.h>
#include <ctype.h>

#include "hoel.h"
#include "h-private.h"

static char * h_get_insert_values_from_json_object(const struct _h_connection * conn, json_t * data) {
  char * new_data = NULL, * escape, * insert_data = NULL, * tmp;
  const char * key = NULL;
  json_t * value = NULL, * raw;
  int i = 0;

  json_object_foreach((json_t *)data, key, value) {
    switch (json_typeof(value)) {
      case JSON_STRING:
        escape = h_escape_string_with_quotes(conn, json_string_value(value));
        if (escape == NULL) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Hoel/h_get_insert_values_from_json_object - Error escape");
          o_free(new_data);
          new_data = NULL;
        } else {
          new_data = msprintf("%s", escape);
          o_free(escape);
        }
        break;
      case JSON_INTEGER:
        new_data = msprintf("%"JSON_INTEGER_FORMAT, json_integer_value(value));
        break;
      case JSON_REAL:
        new_data = msprintf("%f", json_real_value(value));
        break;
      case JSON_TRUE:
        new_data = o_strdup("1");
        break;
      case JSON_FALSE:
        new_data = o_strdup("0");
        break;
      case JSON_NULL:
        new_data = o_strdup("NULL");
        break;
      case JSON_OBJECT:
        raw = json_object_get(value, "raw");
        if (raw != NULL && json_is_string(raw)) {
          new_data = o_strdup(json_string_value(raw));
        } else {
          new_data = o_strdup("NULL");
        }
        break;
      default:
        tmp = json_dumps(value, JSON_ENCODE_ANY);
        y_log_message(Y_LOG_LEVEL_ERROR, "Hoel/h_get_insert_values_from_json_object - Error decoding value %s, inserting NULL value", tmp);
        o_free(tmp);
        new_data = o_strdup("NULL");
        break;
    }
    if (new_data == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Hoel/h_get_insert_values_from_json_object - Error allocating memory for new_data");
      return NULL;
    }
    if (i == 0) {
      insert_data = msprintf("(%s", new_data);
      if (insert_data == NULL) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Hoel/h_get_insert_values_from_json_object - Error allocating insert_data");
      }
      i = 1;
      o_free(new_data);
    } else {
      tmp = msprintf("%s,%s", insert_data, new_data);
      if (tmp == NULL) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Hoel/h_get_insert_values_from_json_object - Error allocating tmp");
        o_free(insert_data);
        o_free(new_data);
        return NULL;
      }
      o_free(insert_data);
      o_free(new_data);
      insert_data = tmp;
    }
  }
  tmp = msprintf("%s)", insert_data);
  o_free(insert_data);
  if (tmp == NULL) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Hoel/h_get_insert_values_from_json_object - Error final allocation for tmp");
  }
  return tmp;
}

static char * h_get_insert_columns_from_json_object(json_t * data) {
  char * insert_cols = NULL, * tmp;
  const char * key = NULL;
  json_t * value = NULL;
  int i = 0;

  json_object_foreach((json_t *)data, key, value) {
    if (i == 0) {
      insert_cols = msprintf("%s", key);
      if (insert_cols == NULL) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Hoel/h_get_insert_columns_from_json_object - Error allocating insert_cols");
        return NULL;
      }
      i = 1;
    } else {
      tmp = msprintf("%s,%s", insert_cols, key);
      o_free(insert_cols);
      if (tmp == NULL) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Hoel/h_get_insert_columns_from_json_object - Error allocating insert_cols");
        return NULL;
      }
      insert_cols = tmp;
    }
  }
  
  return insert_cols;
}

/**
 * Builds an insert query from a json object and a table name
 * Returned value must be o_free'd after use
 */
static char * h_get_insert_query_from_json_object(const struct _h_connection * conn, json_t * data, const char * table) {
  char * to_return = NULL, * insert_cols, * insert_data;
  
  insert_cols = h_get_insert_columns_from_json_object(data);
  insert_data = h_get_insert_values_from_json_object(conn, data);
  if (insert_cols == NULL || insert_data == NULL) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Hoel/h_get_insert_query_from_json_object - Error h_get_insert_columns_from_json_object or h_get_insert_values_from_json_object");
  } else {
    to_return = msprintf("INSERT INTO %s (%s) VALUES %s", table, insert_cols, insert_data);
  }
  o_free(insert_cols);
  o_free(insert_data);
  if (to_return == NULL) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Hoel/h_get_insert_query_from_json_object - Error allocating memory for h_get_insert_query_from_json_object");
  }
  return to_return;
}

/**
 * Builds an insert query from a json object and a table name
 * Returned value must be o_free'd after use
 */
static char * h_get_insert_query_from_json_array(const struct _h_connection * conn, json_t * j_array, const char * table) {
  json_t * j_row = NULL;
  size_t index = 0;
  char * to_return = NULL, * insert_cols, * insert_data, * tmp;
  
  json_array_foreach(j_array, index, j_row) {
    insert_data = h_get_insert_values_from_json_object(conn, j_row);
    if (!index) {
      insert_cols = h_get_insert_columns_from_json_object(j_row);
      to_return = msprintf("INSERT INTO %s (%s) VALUES %s", table, insert_cols, insert_data);
      o_free(insert_cols);
      if (to_return == NULL) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Hoel/h_get_insert_query_from_json_array - Error allocating to_return");
        o_free(insert_data);
        return NULL;
      }
    } else {
      tmp = msprintf("%s,%s", to_return, insert_data);
      if (tmp == NULL) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Hoel/h_get_insert_query_from_json_array - Error allocating tmp");
        o_free(insert_data);
        return NULL;
      }
      o_free(to_return);
      to_return = tmp;
    }
    o_free(insert_data);
  }
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
 * the returned value must be o_free'd after use
 */
static char * h_get_where_clause_from_json_object(const struct _h_connection * conn, const json_t * where) {
  const char * key = NULL;
  json_t * value = NULL, * ope, * val, * j_element;
  char * where_clause = NULL, * dump = NULL, * escape = NULL, * tmp, * clause = NULL, * dump2 = NULL;
  int i = 0;
  size_t index = 0;
  
  if (conn == NULL) {
    y_log_message(Y_LOG_LEVEL_DEBUG, "Hoel/h_get_where_clause_from_json_object - Error conn is NULL");
    return NULL;
  } else if (where == NULL || (json_is_object(where) && json_object_size(where) == 0)) {
    return o_strdup("1=1");
  } else {
    json_object_foreach((json_t *)where, key, value) {
      if (!json_is_string(value) && !json_is_real(value) && !json_is_integer(value) && !json_is_object(value) && !json_is_null(value)) {
        dump = json_dumps(value, JSON_ENCODE_ANY);
        y_log_message(Y_LOG_LEVEL_DEBUG, "Hoel/h_get_where_clause_from_json_object - Error where value is invalid: %s", dump);
        o_free(dump);
        return NULL;
      } else {
        if (json_is_object(value)) {
          ope = json_object_get(value, "operator");
          val = json_object_get(value, "value");
          if (ope == NULL ||
              !json_is_string(ope) ||
              (val == NULL && 0 != o_strcasecmp("NOT NULL", json_string_value(ope))) ||
              (!json_is_string(val) && !json_is_real(val) && !json_is_integer(val) && 0 != o_strcasecmp("NOT NULL", json_string_value(ope)) && 0 != o_strcasecmp("IN", json_string_value(ope)))) {
            dump = json_dumps(val, JSON_ENCODE_ANY);
            dump2 = json_dumps(ope, JSON_ENCODE_ANY);
            y_log_message(Y_LOG_LEVEL_DEBUG, "Hoel/h_get_where_clause_from_json_object - Error where object value is invalid: %s %s", dump, dump2);
            o_free(dump);
            o_free(dump2);
            o_free(where_clause);
            return NULL;
          } else {
            if (0 == o_strcasecmp("NOT NULL", json_string_value(ope))) {
              clause = msprintf("%s IS NOT NULL", key);
            } else if (0 == o_strcasecmp("raw", json_string_value(ope)) && json_is_string(val)) {
              clause = msprintf("%s %s", key, json_string_value(val));
            } else if (0 == o_strcasecmp("IN", json_string_value(ope))) {
              if (json_is_array(val) && json_array_size(val) > 0) {
                clause = NULL, tmp = NULL;
                json_array_foreach(val, index, j_element) {
                  if (!json_is_string(j_element) && !json_is_real(j_element) && !json_is_integer(j_element)) {
                    o_free(clause);
                    o_free(where_clause);
                    y_log_message(Y_LOG_LEVEL_ERROR, "Hoel - Error element value in IN statement array must be real, integer or string");
                    return NULL;
                  } else {
                    if (json_is_string(j_element)) {
                      escape = h_escape_string_with_quotes(conn, json_string_value(j_element));
                      if (escape == NULL) {
                        y_log_message(Y_LOG_LEVEL_ERROR, "Hoel - Error escape");
                        o_free(clause);
                        o_free(where_clause);
                        return NULL;
                      }
                      dump = msprintf("%s", escape);
                      o_free(escape);
                    } else if (json_is_real(j_element)) {
                      dump = msprintf("%f", json_real_value(j_element));
                    } else {
                      dump = msprintf("%" JSON_INTEGER_FORMAT, json_integer_value(j_element));
                    }
                    if (clause == NULL) {
                      clause = msprintf("%s IN (%s", key, dump);
                    } else {
                      tmp = msprintf("%s,%s", clause, dump);
                      o_free(clause);
                      clause = tmp;
                    }
                    o_free(dump);
                  }
                }
                tmp = msprintf("%s)", clause);
                o_free(clause);
                clause = tmp;
              } else {
                o_free(where_clause);
                y_log_message(Y_LOG_LEVEL_ERROR, "Hoel - Error value in IN statement must be a non empty JSON array");
                return NULL;
              }
            } else {
              if (json_is_real(val)) {
                clause = msprintf("%s %s %f", key, json_string_value(ope), json_real_value(val));
              } else if (json_is_integer(val)) {
                clause = msprintf("%s %s %" JSON_INTEGER_FORMAT, key, json_string_value(ope), json_integer_value(val));
              } else {
                escape = h_escape_string_with_quotes(conn, json_string_value(val));
                if (escape == NULL) {
                  y_log_message(Y_LOG_LEVEL_ERROR, "Hoel - Error escape");
                  o_free(where_clause);
                  return NULL;
                }
                clause = msprintf("%s %s %s", key, json_string_value(ope), escape);
                o_free(escape);
              }
            }
            if (clause == NULL) {
              y_log_message(Y_LOG_LEVEL_ERROR, "Hoel - Error allocating memory for clause");
              o_free(where_clause);
              return NULL;
            }
          }
        } else {
          if (json_is_null(value)) {
            clause = msprintf("%s IS NULL", key);
          } else if (json_is_string(value)) {
            escape = h_escape_string_with_quotes(conn, json_string_value(value));
            if (escape == NULL) {
              y_log_message(Y_LOG_LEVEL_ERROR, "Hoel - Error escape");
              o_free(where_clause);
              return NULL;
            }
            clause = msprintf("%s=%s", key, escape);
            o_free(escape);
          } else if (json_is_integer(value)) {
            clause = msprintf("%s='%"JSON_INTEGER_FORMAT"'", key, json_integer_value(value));
          } else if (json_is_real(value)) {
            clause = msprintf("%s='%f'", key, json_real_value(value));
          } else if (json_is_true(value)) {
            clause = msprintf("%s=1");
          } else if (json_is_false(value)) {
            clause = msprintf("%s=0");
          }
          if (clause == NULL) {
            y_log_message(Y_LOG_LEVEL_ERROR, "Hoel - Error allocating memory for clause");
            o_free(where_clause);
            return NULL;
          }
        }
        if (i == 0) {
          where_clause = o_strdup(clause);
          if (where_clause == NULL) {
            y_log_message(Y_LOG_LEVEL_DEBUG, "Hoel/h_get_where_clause_from_json_object - Error where_clause");
            o_free(clause);
            o_free(where_clause);
            return NULL;
          }
          o_free(clause);
          i = 1;
        } else {
          tmp = msprintf("%s AND %s", where_clause, clause);
          o_free(where_clause);
          if (tmp == NULL) {
            y_log_message(Y_LOG_LEVEL_DEBUG, "Hoel/h_get_where_clause_from_json_object - Error tmp where_clause");
            o_free(clause);
            o_free(where_clause);
            return NULL;
          }
          o_free(clause);
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
 * the returned value must be o_free'd after use
 */
static char * h_get_set_clause_from_json_object(const struct _h_connection * conn, const json_t * set) {
  const char * key = NULL;
  json_t * value = NULL, * raw;
  char * where_clause = NULL, * escape = NULL, * tmp;
  int i = 0;
  
  if (conn == NULL || set == NULL || !json_is_object(set)) {
    y_log_message(Y_LOG_LEVEL_DEBUG, "Hoel/h_get_set_clause_from_json_object - Error null input parameters");
    return NULL;
  } else {
    json_object_foreach((json_t *)set, key, value) {
      if (!json_is_string(value) && !json_is_real(value) && !json_is_integer(value) && !json_is_null(value) && !json_is_object(value)) {
        tmp = json_dumps(value, JSON_ENCODE_ANY);
        y_log_message(Y_LOG_LEVEL_DEBUG, "Hoel/h_get_set_clause_from_json_object - Error value invalid: %s", tmp);
        o_free(tmp);
        o_free(where_clause);
        return NULL;
      } else {
        if (json_is_string(value)) {
          tmp = h_escape_string_with_quotes(conn, json_string_value(value));
          escape = msprintf("%s", tmp);
          o_free(tmp);
        } else if (json_is_real(value)) {
          escape = msprintf("%f", json_real_value(value));
        } else if (json_is_integer(value)) {
          escape = msprintf("%" JSON_INTEGER_FORMAT, json_integer_value(value));
        } else if (json_is_object(value)) {
          raw = json_object_get(value, "raw");
          if (raw != NULL && json_is_string(raw)) {
            escape = o_strdup(json_string_value(raw));
          } else {
            escape = o_strdup("NULL");
          }
        } else {
          escape = o_strdup("");
        }
        if (escape == NULL) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Hoel - Error escape");
          return NULL;
        }
        if (i == 0) {
          if (!json_is_null(value)) {
            where_clause = msprintf("%s=%s", key, escape);
          } else {
            where_clause = msprintf("%s=NULL", key);
          }
          if (where_clause == NULL) {
            y_log_message(Y_LOG_LEVEL_DEBUG, "Hoel/h_get_set_clause_from_json_object - Error where_clause");
            return NULL;
          }
          i = 1;
        } else {
          if (!json_is_null(value)) {
            tmp = msprintf("%s, %s=%s", where_clause, key, escape);
          } else {
            tmp = msprintf("%s, %s=NULL", where_clause, key);
          }
          o_free(where_clause);
          if (tmp == NULL) {
            y_log_message(Y_LOG_LEVEL_DEBUG, "Hoel/h_get_set_clause_from_json_object - Error tmp where_clause");
            return NULL;
          }
          where_clause = tmp;
        }
        o_free(escape);
      }
    }
    return where_clause;
  }
}

/**
 * h_select
 * Execute a select query
 * Uses a json_t * parameter for the query parameters
 * Store the result of the query in j_result if specified. j_result must be decref'd after use
 * Duplicate the generated query in generated_query if specified, must be o_free'd after use
 * return H_OK on success
 */
int h_select(const struct _h_connection * conn, const json_t * j_query, json_t ** j_result, char ** generated_query) {
  const char * table;
  const json_t * cols, * where, * order_by;
  json_int_t limit, offset;
  char * query, * columns = NULL, * where_clause = NULL, * tmp, * str_where_limit,  * str_order_by;
  const char * col;
  size_t index = 0;
  json_t * value;
  int res;

  if (conn == NULL || j_result == NULL || j_query == NULL || !json_is_object(j_query) || json_object_get(j_query, "table") == NULL || !json_is_string(json_object_get(j_query, "table"))) {
    y_log_message(Y_LOG_LEVEL_DEBUG, "Hoel/h_select Error invalid input parameters");
    return H_ERROR_PARAMS;
  }
  
  table = json_string_value((const json_t *)json_object_get(j_query, "table"));
  cols = json_object_get(j_query, "columns");
  where = json_object_get(j_query, "where");
  order_by = json_object_get(j_query, "order_by");
  limit = json_is_integer(json_object_get(j_query, "limit"))?json_integer_value(json_object_get(j_query, "limit")):0;
  offset = json_is_integer(json_object_get(j_query, "offset"))?json_integer_value(json_object_get(j_query, "offset")):0;
  
  where_clause = h_get_where_clause_from_json_object(conn, (json_t *)where);
  if (where_clause == NULL) {
    y_log_message(Y_LOG_LEVEL_DEBUG, "Hoel/h_select Error where_clause construction");
    return H_ERROR_PARAMS;
  }
  
  if (cols == NULL) {
    columns = o_strdup("*");
  } else if (json_is_array(cols)) {
    json_array_foreach(cols, index, value) {
      if (json_is_string(value)) {
        col = json_string_value(value);
        if (col == NULL) {
          y_log_message(Y_LOG_LEVEL_DEBUG, "Hoel/h_select Error col");
          o_free(where_clause);
          o_free(columns);
          return H_ERROR_MEMORY;
        }
      } else {
        y_log_message(Y_LOG_LEVEL_DEBUG, "Hoel/h_select Error column not string");
        o_free(where_clause);
        return H_ERROR_PARAMS;
      }
      if (index == 0) {
        columns = o_strdup(col);
        if (columns == NULL) {
          y_log_message(Y_LOG_LEVEL_DEBUG, "Hoel/h_select Error allocating columns");
          o_free(where_clause);
          return H_ERROR_MEMORY;
        }
      } else {
        tmp = msprintf("%s, %s", columns, col);
        if (tmp == NULL) {
          y_log_message(Y_LOG_LEVEL_DEBUG, "Hoel/h_select Error allocating clause");
          o_free(where_clause);
          o_free(columns);
          return H_ERROR_MEMORY;
        }
        o_free(columns);
        columns = tmp;
      }
    }
  } else {
    y_log_message(Y_LOG_LEVEL_DEBUG, "Hoel/h_select Error cols not array");
    o_free(where_clause);
    return H_ERROR_PARAMS;
  }
  
  if (columns == NULL) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Hoel - Error allocating memory for columns");
    o_free(where_clause);
    return H_ERROR_MEMORY;
  }
  
  if (limit > 0) {
    if (offset > 0) {
      str_where_limit = msprintf("LIMIT %" JSON_INTEGER_FORMAT " OFFSET %" JSON_INTEGER_FORMAT, limit, offset);
    } else {
      str_where_limit = msprintf("LIMIT %" JSON_INTEGER_FORMAT, limit);
    }
    if (str_where_limit == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Hoel - Error allocating memory for str_where_limit");
      o_free(columns);
      o_free(where_clause);
      return H_ERROR_MEMORY;
    }
  } else {
    str_where_limit = o_strdup("");
  }
  if (order_by != NULL && json_is_string(order_by)) {
    str_order_by = msprintf("ORDER BY %s", json_string_value(order_by));
  } else {
    str_order_by = o_strdup("");
  }
  if (str_order_by == NULL) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Hoel - Error allocating memory for str_order_by");
    o_free(columns);
    o_free(where_clause);
    o_free(str_where_limit);
    return H_ERROR_MEMORY;
  }
  
  if (str_order_by == NULL) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Hoel - Error allocating memory for str_order_by");
    o_free(columns);
    o_free(where_clause);
    o_free(str_where_limit);
    o_free(str_order_by);
    return H_ERROR_MEMORY;
  }
  
  query = msprintf("SELECT %s FROM %s WHERE %s %s %s", columns, table, where_clause, str_order_by, str_where_limit);
  if (query == NULL) {
    y_log_message(Y_LOG_LEVEL_DEBUG, "Hoel/h_select Error allocating query");
    o_free(columns);
    o_free(where_clause);
    o_free(str_where_limit);
    o_free(str_order_by);
    return H_ERROR_MEMORY;
  } else {
    if (generated_query != NULL) {
      *generated_query = o_strdup(query);
    }
    res = h_query_select_json(conn, query, j_result);
    o_free(columns);
    o_free(where_clause);
    o_free(str_where_limit);
    o_free(str_order_by);
    o_free(query);
    return res;
  }
}

/**
 * h_insert
 * Execute an insert query
 * Uses a json_t * parameter for the query parameters
 * Duplicate the generated query in generated_query if specified, must be o_free'd after use
 * return H_OK on success
 */
int h_insert(const struct _h_connection * conn, const json_t * j_query, char ** generated_query) {
  const char * table;
  char * query;
  json_t * values;
  int res;
  
  if (conn != NULL && j_query != NULL && json_is_object(j_query) && json_is_string(json_object_get(j_query, "table")) && (json_is_object(json_object_get(j_query, "values")) || json_is_array(json_object_get(j_query, "values")))) {
    /* Construct query */
    table = json_string_value((const json_t *)json_object_get(j_query, "table"));
    values = json_object_get(j_query, "values");
    switch (json_typeof(values)) {
      case JSON_OBJECT:
        query = h_get_insert_query_from_json_object(conn, values, table);
        if (query != NULL) {
          if (generated_query != NULL) {
            *generated_query = o_strdup(query);
          }
          res = h_query_insert(conn, query);
          o_free(query);
          if (res != H_OK) {
            y_log_message(Y_LOG_LEVEL_DEBUG, "Hoel/h_insert - Error executing query (1)");
            return H_ERROR_QUERY;
          }
          return res;
        } else {
          y_log_message(Y_LOG_LEVEL_DEBUG, "Hoel/h_insert - Error allocating query (1)");
          return H_ERROR_MEMORY;
        }
        break;
      case JSON_ARRAY:
        if (json_array_size(values)) {
          query = h_get_insert_query_from_json_array(conn, values, table);
          if (query != NULL) {
            if (generated_query != NULL) {
              /* Export just the first query */
              *generated_query = o_strdup(query);
            }
            res = h_query_insert(conn, query);
            o_free(query);
            if (res != H_OK) {
              y_log_message(Y_LOG_LEVEL_DEBUG, "Hoel/h_insert - Error executing query (2)");
              return H_ERROR_QUERY;
            }
            return res;
          } else {
            y_log_message(Y_LOG_LEVEL_DEBUG, "Hoel/h_insert - Error allocating query (2)");
            return H_ERROR_MEMORY;
          }
        } else {
          y_log_message(Y_LOG_LEVEL_DEBUG, "Hoel/h_insert - Error no values to insert");
          return H_ERROR_QUERY;
        }
        break;
      default:
        y_log_message(Y_LOG_LEVEL_DEBUG, "Hoel/h_insert - Error unknown object type for values");
        return H_ERROR_PARAMS;
        break;
    }
  } else {
    y_log_message(Y_LOG_LEVEL_DEBUG, "Hoel/h_insert - Error null input parameters");
    return H_ERROR_PARAMS;
  }
}

/**
 * h_last_insert_id
 * return the id of the last inserted value
 * return a pointer to json_t * on success, NULL otherwise.
 * The returned value is of type JSON_INTEGER
 */
json_t * h_last_insert_id(const struct _h_connection * conn) {
  json_t * j_data = NULL;
  if (conn != NULL && conn->connection != NULL) {
    if (0) {
      /* Not happening */
#ifdef _HOEL_SQLITE
    } else if (conn->type == HOEL_DB_TYPE_SQLITE) {
      int last_id = h_last_insert_id_sqlite(conn);
      if (last_id > 0) {
        j_data = json_integer(last_id);
      }
#endif
#ifdef _HOEL_MARIADB
    } else if (conn->type == HOEL_DB_TYPE_MARIADB) {
      int last_id = h_last_insert_id_mariadb(conn);
      if (last_id > 0) {
        j_data = json_integer(last_id);
      }
#endif
#ifdef _HOEL_PGSQL
    } else if (conn->type == HOEL_DB_TYPE_PGSQL) {
      int last_id = h_last_insert_id_pgsql(conn);
      if (last_id > 0) {
        j_data = json_integer(last_id);
      }
#endif
    }
  }
  return j_data;
}

/**
 * h_update
 * Execute an update query
 * Uses a json_t * parameter for the query parameters
 * Duplicate the generated query in generated_query if specified, must be o_free'd after use
 * return H_OK on success
 */
int h_update(const struct _h_connection * conn, const json_t * j_query, char ** generated_query) {
  char * set_clause, * where_clause, * query;
  const char * table;
  int res;
  json_t * set, * where;
  
  if (j_query == NULL || !json_is_object(j_query) || !json_is_string(json_object_get(j_query, "table")) || !json_is_object(json_object_get(j_query, "set"))) {
    y_log_message(Y_LOG_LEVEL_DEBUG, "Hoel/h_update - Error invalid input parameters");
    return H_ERROR_PARAMS;
  }
  
  table = json_string_value((const json_t *)json_object_get(j_query, "table"));
  
  set = json_object_get(j_query, "set");
  set_clause = h_get_set_clause_from_json_object(conn, set);
  
  if (set_clause == NULL) {
    y_log_message(Y_LOG_LEVEL_DEBUG, "Hoel/h_update - Error generating set clause");
    return H_ERROR_PARAMS;
  }
  
  if (json_is_object(json_object_get(j_query, "where")) && json_object_size(json_object_get(j_query, "where")) > 0) {
    where = json_object_get(j_query, "where");
    where_clause = h_get_where_clause_from_json_object(conn, where);
    query = msprintf("UPDATE %s SET %s WHERE %s", table, set_clause, where_clause);
    o_free(where_clause);
  } else {
    query = msprintf("UPDATE %s SET %s", table, set_clause);
  }
  
  o_free(set_clause);
  if (query == NULL) {
    y_log_message(Y_LOG_LEVEL_DEBUG, "Hoel/h_update - Error allocating query");
    return H_ERROR_MEMORY;
  }
  if (generated_query != NULL) {
    *generated_query = o_strdup(query);
  }
  res = h_query_update(conn, query);
  o_free(query);
  return res;
}

/**
 * h_delete
 * Execute a delete query
 * Uses a json_t * parameter for the query parameters
 * Duplicate the generated query in generated_query if specified, must be o_free'd after use
 * return H_OK on success
 */
int h_delete(const struct _h_connection * conn, const json_t * j_query, char ** generated_query) {
  char * where_clause, * query;
  const char * table;
  int res;
  json_t * where;
  
  if (j_query == NULL || !json_is_object(j_query) || !json_is_string(json_object_get(j_query, "table"))) {
    y_log_message(Y_LOG_LEVEL_DEBUG, "Hoel/h_delete - Error invalid input parameters");
    return H_ERROR_PARAMS;
  }
  
  table = json_string_value((json_t *)json_object_get(j_query, "table"));
  
  if (json_is_object(json_object_get(j_query, "where")) && json_object_size(json_object_get(j_query, "where")) > 0) {
    where = json_object_get(j_query, "where");
    where_clause = h_get_where_clause_from_json_object(conn, where);
    
    if (where_clause == NULL) {
      y_log_message(Y_LOG_LEVEL_DEBUG, "Hoel/h_delete - Error invalid input parameters");
      return H_ERROR_PARAMS;
    }
    query = msprintf("DELETE FROM %s WHERE %s", table, where_clause);
    o_free(where_clause);
  } else {
    query = msprintf("DELETE FROM %s", table);
  }
  
  if (query == NULL) {
    y_log_message(Y_LOG_LEVEL_DEBUG, "Hoel/h_delete - Error allocating query");
    return H_ERROR_MEMORY;
  }
  if (generated_query != NULL) {
    *generated_query = o_strdup(query);
  }
  res = h_query_delete(conn, query);
  o_free(query);
  return res;
}
