/* Public domain, no copyright. Use at your own risk. */
/* only sqlite3 backend is tested, I will assume the */
/* behaviour is the same with other backends */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <jansson.h>

#include <check.h>
#ifndef  _HOEL_SQLITE
  #define _HOEL_SQLITE
#endif
#include "hoel.h"

#define DEFAULT_BD_PATH "/tmp/test.db"
#define WRONG_BD_PATH "nope.db"

#define UNSAFE_STRING "un'safe' (\"string\")#!/$%*];"

#define SELECT_DATA_1 "SELECT integer_col, double_col, string_col, date_col FROM test_table WHERE integer_col = 1"
#define SELECT_DATA_2 "SELECT integer_col, double_col, string_col, date_col FROM test_table WHERE integer_col = 2"
#define SELECT_DATA_ERROR "SELECT integer_col, double_col, string_col, date_col FROM test_table WHERE integer_col = 'error'"
#define SELECT_DATA_ALL "SELECT * FROM test_table"

#define INSERT_DATA_1 "INSERT INTO test_table (integer_col, double_col, string_col, date_col) VALUES (1, 4.2, 'value1', date('now'))"
#define INSERT_DATA_2 "INSERT INTO test_table (integer_col, double_col, string_col, date_col) VALUES (2, 5.4, 'value2', strftime('%s','2016-06-22 00:52:56'))"
#define INSERT_DATA_ERROR "INSERT INTO test_table (integer_col, double_col, string_col, date_col) VALUES ('error', 'error_double', 'value error', date('now'))"

#define DELETE_DATA_1 "DELETE FROM test_table WHERE integer_col = 1"
#define DELETE_DATA_2 "DELETE FROM test_table WHERE integer_col = 2"
#define DELETE_DATA_ERROR "DELETE FROM test_table WHERE wrong_table = 1"
#define DELETE_DATA_ALL "DELETE FROM test_table"

#define UPDATE_DATA_1 "UPDATE test_table SET string_col='new value1', double_col=5.4 WHERE integer_col = 1"

#define WHERE_CLAUSE_NO_FORMAT "And then, nothing happened"

void print_result(struct _h_result result) {
  size_t col, row, i;
  printf("rows: %u, col: %u\n", result.nb_rows, result.nb_columns);
  for (row = 0; row<result.nb_rows; row++) {
    for (col=0; col<result.nb_columns; col++) {
      switch(result.data[row][col].type) {
        case HOEL_COL_TYPE_INT:
          printf("| %lld ", ((struct _h_type_int *)result.data[row][col].t_data)->value);
          break;
        case HOEL_COL_TYPE_DOUBLE:
          printf("| %f ", ((struct _h_type_double *)result.data[row][col].t_data)->value);
          break;
        case HOEL_COL_TYPE_TEXT:
          printf("| %s ", ((struct _h_type_text *)result.data[row][col].t_data)->value);
          break;
        case HOEL_COL_TYPE_BLOB:
          for (i=0; i<((struct _h_type_blob *)result.data[row][col].t_data)->length; i++) {
            printf("%c", *((char*)(((struct _h_type_blob *)result.data[row][col].t_data)->value+i)));
            if (i%80 == 0 && i>0) {
              printf("\n");
            }
          }
          break;
        case HOEL_COL_TYPE_NULL:
          printf("| null ");
          break;
      }
    }
    printf("|\n");
  }
}

START_TEST(test_hoel_init)
{
  struct _h_connection * conn;
  conn = h_connect_sqlite(DEFAULT_BD_PATH);
  ck_assert_ptr_ne(conn, NULL);
  ck_assert_ptr_eq(h_connect_sqlite(WRONG_BD_PATH), NULL);
  ck_assert_int_eq(h_close_db(conn), H_OK);
  ck_assert_int_ne(h_close_db(NULL), H_OK);
  ck_assert_int_eq(h_clean_connection(conn), H_OK);
  ck_assert_int_ne(h_clean_connection(NULL), H_OK);
}
END_TEST

START_TEST(test_hoel_escape_string)
{
  struct _h_connection * conn;
  char * escaped;
  json_t * j_query, * j_result;
  int res;
  
  conn = h_connect_sqlite(DEFAULT_BD_PATH);
  ck_assert_ptr_ne(conn, NULL);
  escaped = h_escape_string(conn, "value");
  ck_assert_str_eq(escaped, "value");
  h_free(escaped);
  escaped = h_escape_string(conn, "unsafe ' value\"!");
  ck_assert_str_eq(escaped, "unsafe '' value\"!");
  h_free(escaped);
  
  j_query = json_pack("{sss{siss}}", "table", "test_table", "values", "integer_col", 666, "string_col", UNSAFE_STRING);
  res = h_insert(conn, j_query, NULL);
  json_decref(j_query);
  ck_assert_int_eq(res, H_OK);
  j_query = json_pack("{sss[s]s{si}}", "table", "test_table", "columns", "string_col", "where", "integer_col", 666);
  res = h_select(conn, j_query, &j_result, NULL);
  json_decref(j_query);
  ck_assert_int_eq(res, H_OK);
  ck_assert_str_eq(UNSAFE_STRING, json_string_value(json_object_get(json_array_get(j_result, 0), "string_col")));
  json_decref(j_result);
  
  ck_assert_int_eq(h_close_db(conn), H_OK);
  ck_assert_int_eq(h_clean_connection(conn), H_OK);
}
END_TEST

START_TEST(test_hoel_escape_string_with_quotes)
{
  struct _h_connection * conn;
  char * escaped;
  json_t * j_query, * j_result;
  int res;
  
  conn = h_connect_sqlite(DEFAULT_BD_PATH);
  ck_assert_ptr_ne(conn, NULL);
  escaped = h_escape_string_with_quotes(conn, "value");
  ck_assert_str_eq(escaped, "'value'");
  h_free(escaped);
  escaped = h_escape_string_with_quotes(conn, "unsafe ' value\"!");
  ck_assert_str_eq(escaped, "'unsafe '' value\"!'");
  h_free(escaped);
  
  j_query = json_pack("{sss{siss}}", "table", "test_table", "values", "integer_col", 666, "string_col", UNSAFE_STRING);
  res = h_insert(conn, j_query, NULL);
  json_decref(j_query);
  ck_assert_int_eq(res, H_OK);
  j_query = json_pack("{sss[s]s{si}}", "table", "test_table", "columns", "string_col", "where", "integer_col", 666);
  res = h_select(conn, j_query, &j_result, NULL);
  json_decref(j_query);
  ck_assert_int_eq(res, H_OK);
  ck_assert_str_eq(UNSAFE_STRING, json_string_value(json_object_get(json_array_get(j_result, 0), "string_col")));
  json_decref(j_result);
  
  ck_assert_int_eq(h_close_db(conn), H_OK);
  ck_assert_int_eq(h_clean_connection(conn), H_OK);
}
END_TEST

START_TEST(test_hoel_insert)
{
  struct _h_connection * conn;
  struct _h_result result;
  struct _h_data * last_id;
  conn = h_connect_sqlite(DEFAULT_BD_PATH);
  ck_assert_ptr_ne(conn, NULL);
  ck_assert_int_eq(h_query_delete(conn, DELETE_DATA_ALL), H_OK);
  ck_assert_int_eq(h_query_insert(conn, INSERT_DATA_1), H_OK);
  last_id = h_query_last_insert_id(conn);
  ck_assert_int_eq(last_id->type, HOEL_COL_TYPE_INT);
  ck_assert_int_gt(((struct _h_type_int *)last_id->t_data)->value, 0);
  ck_assert_int_eq(h_clean_data(last_id), H_OK);
  h_free(last_id);
  ck_assert_ptr_ne(last_id, NULL);
  ck_assert_int_eq(h_query_insert(conn, NULL), H_ERROR_PARAMS);
  ck_assert_int_eq(h_query_select(conn, SELECT_DATA_1, &result), H_OK);
  ck_assert_int_eq(result.nb_rows, 1);
  ck_assert_int_eq(result.nb_columns, 4);
  ck_assert_int_eq(result.data[0][0].type, HOEL_COL_TYPE_INT);
  ck_assert_int_eq(((struct _h_type_int *)result.data[0][0].t_data)->value, 1);
  ck_assert_int_eq(result.data[0][1].type, HOEL_COL_TYPE_DOUBLE);
  ck_assert_double_eq(((struct _h_type_double *)result.data[0][1].t_data)->value, 4.2);
  ck_assert_int_eq(result.data[0][2].type, HOEL_COL_TYPE_TEXT);
  ck_assert_str_eq(((struct _h_type_text *)result.data[0][2].t_data)->value, "value1");
  ck_assert_int_eq(h_clean_result(&result), H_OK);
  ck_assert_int_eq(h_query_select(conn, SELECT_DATA_ERROR, &result), H_OK);
  ck_assert_int_eq(result.nb_rows, 0);
  ck_assert_int_eq(result.nb_columns, 4);
  ck_assert_int_eq(h_clean_result(&result), H_OK);
  ck_assert_int_eq(h_query_delete(conn, DELETE_DATA_1), H_OK);
  ck_assert_int_eq(h_close_db(conn), H_OK);
  ck_assert_int_eq(h_clean_connection(conn), H_OK);
}
END_TEST

START_TEST(test_hoel_update)
{
  struct _h_connection * conn;
  struct _h_result result;
  conn = h_connect_sqlite(DEFAULT_BD_PATH);
  ck_assert_ptr_ne(conn, NULL);
  ck_assert_int_eq(h_query_insert(conn, INSERT_DATA_1), H_OK);
  ck_assert_int_eq(h_query_insert(conn, NULL), H_ERROR_PARAMS);
  ck_assert_int_eq(h_query_select(conn, SELECT_DATA_1, &result), H_OK);
  ck_assert_int_eq(result.nb_rows, 1);
  ck_assert_int_eq(result.nb_columns, 4);
  ck_assert_int_eq(((struct _h_type_int *)result.data[0][0].t_data)->value, 1);
  ck_assert_double_eq(((struct _h_type_double *)result.data[0][1].t_data)->value, 4.2);
  ck_assert_str_eq(((struct _h_type_text *)result.data[0][2].t_data)->value, "value1");
  ck_assert_int_eq(h_clean_result(&result), H_OK);
  ck_assert_int_eq(h_query_update(conn, UPDATE_DATA_1), H_OK);
  ck_assert_int_eq(h_query_select(conn, SELECT_DATA_1, &result), H_OK);
  ck_assert_int_eq(result.nb_rows, 1);
  ck_assert_int_eq(result.nb_columns, 4);
  ck_assert_int_eq(((struct _h_type_int *)result.data[0][0].t_data)->value, 1);
  ck_assert_double_eq(((struct _h_type_double *)result.data[0][1].t_data)->value, 5.4);
  ck_assert_str_eq(((struct _h_type_text *)result.data[0][2].t_data)->value, "new value1");
  ck_assert_int_eq(h_clean_result(&result), H_OK);
  ck_assert_int_eq(h_query_delete(conn, DELETE_DATA_1), H_OK);
  ck_assert_int_eq(h_close_db(conn), H_OK);
  ck_assert_int_eq(h_clean_connection(conn), H_OK);
}
END_TEST

START_TEST(test_hoel_delete)
{
  struct _h_connection * conn;
  struct _h_result result;
  conn = h_connect_sqlite(DEFAULT_BD_PATH);
  ck_assert_ptr_ne(conn, NULL);
  ck_assert_int_eq(h_query_insert(conn, INSERT_DATA_1), H_OK);
  ck_assert_int_eq(h_query_insert(conn, INSERT_DATA_2), H_OK);
  ck_assert_int_eq(h_query_select(conn, SELECT_DATA_1, &result), H_OK);
  ck_assert_int_eq(result.nb_rows, 1);
  ck_assert_int_eq(result.nb_columns, 4);
  ck_assert_int_eq(result.data[0][0].type, HOEL_COL_TYPE_INT);
  ck_assert_int_eq(((struct _h_type_int *)result.data[0][0].t_data)->value, 1);
  ck_assert_int_eq(result.data[0][1].type, HOEL_COL_TYPE_DOUBLE);
  ck_assert_double_eq(((struct _h_type_double *)result.data[0][1].t_data)->value, 4.2);
  ck_assert_int_eq(result.data[0][2].type, HOEL_COL_TYPE_TEXT);
  ck_assert_str_eq(((struct _h_type_text *)result.data[0][2].t_data)->value, "value1");
  ck_assert_int_eq(h_clean_result(&result), H_OK);
  ck_assert_int_eq(h_query_select(conn, SELECT_DATA_2, &result), H_OK);
  ck_assert_int_eq(result.nb_rows, 1);
  ck_assert_int_eq(result.nb_columns, 4);
  ck_assert_int_eq(result.data[0][0].type, HOEL_COL_TYPE_INT);
  ck_assert_int_eq(((struct _h_type_int *)result.data[0][0].t_data)->value, 2);
  ck_assert_int_eq(result.data[0][1].type, HOEL_COL_TYPE_DOUBLE);
  ck_assert_double_eq(((struct _h_type_double *)result.data[0][1].t_data)->value, 5.4);
  ck_assert_int_eq(result.data[0][2].type, HOEL_COL_TYPE_TEXT);
  ck_assert_str_eq(((struct _h_type_text *)result.data[0][2].t_data)->value, "value2");
  ck_assert_int_eq(h_clean_result(&result), H_OK);
  ck_assert_int_eq(h_query_delete(conn, DELETE_DATA_1), H_OK);
  ck_assert_int_eq(h_query_select(conn, SELECT_DATA_ALL, &result), H_OK);
  ck_assert_int_eq(result.nb_rows, 1);
  ck_assert_int_eq(h_clean_result(&result), H_OK);
  ck_assert_int_eq(h_query_delete(conn, DELETE_DATA_2), H_OK);
  ck_assert_int_eq(h_query_select(conn, SELECT_DATA_ALL, &result), H_OK);
  ck_assert_int_eq(result.nb_rows, 0);
  ck_assert_int_eq(h_clean_result(&result), H_OK);
  ck_assert_int_eq(h_close_db(conn), H_OK);
  ck_assert_int_eq(h_clean_connection(conn), H_OK);
}
END_TEST

START_TEST(test_hoel_json_insert)
{
  struct _h_connection * conn;
  char * str_query = NULL;
  json_t * j_query = json_pack("{sss{sisss{ss}sf}}",
                               "table",
                               "test_table",
                               "values",
                                 "integer_col", 1,
                                 "string_col", "value1",
                                 "date_col",
                                   "raw",
                                   "date('now')",
                                  "double_col", 4.2),
          * j_result = NULL;
  conn = h_connect_sqlite(DEFAULT_BD_PATH);
  ck_assert_ptr_ne(conn, NULL);
  ck_assert_int_eq(h_insert(conn, j_query, &str_query), H_OK);
  json_decref(j_query);
  j_query = json_pack("{sss{si}}",
                      "table",
                      "test_table",
                      "where",
                        "integer_col", 1);
  ck_assert_int_eq(h_select(conn, j_query, &j_result, NULL), H_OK);
  ck_assert_ptr_ne(j_result, NULL);
  ck_assert_int_eq(json_is_array(j_result), 1);
  ck_assert_int_eq(json_array_size(j_result), 1);
  ck_assert_str_eq(json_string_value(json_object_get(json_array_get(j_result, 0), "string_col")), "value1");
  ck_assert_int_eq(json_integer_value(json_object_get(json_array_get(j_result, 0), "integer_col")), 1);
  ck_assert_double_eq(json_real_value(json_object_get(json_array_get(j_result, 0), "double_col")), 4.2);
  json_decref(j_result);
  ck_assert_int_eq(o_strlen(str_query), o_strlen("INSERT INTO test_table (integer_col,string_col,date_col,double_col) VALUES (1,'value1',date('now'),4.200000)"));
  h_free(str_query);
  ck_assert_int_eq(h_delete(conn, j_query, NULL), H_OK);
  json_decref(j_query);
  ck_assert_int_eq(h_close_db(conn), H_OK);
  ck_assert_int_eq(h_clean_connection(conn), H_OK);
}
END_TEST

START_TEST(test_hoel_json_update)
{
  struct _h_connection * conn;
  char * str_query = NULL;
  json_t * j_query = json_pack("{sss{sisss{ss}sf}}",
                               "table",
                               "test_table",
                               "values",
                                 "integer_col", 1,
                                 "string_col", "value1",
                                 "date_col",
                                   "raw",
                                   "date('now')",
                                  "double_col", 4.2),
         * j_result = NULL;
  conn = h_connect_sqlite(DEFAULT_BD_PATH);
  ck_assert_ptr_ne(conn, NULL);
  ck_assert_int_eq(h_insert(conn, j_query, NULL), H_OK);
  json_decref(j_query);
  j_query = json_pack("{sss{si}}",
                      "table",
                      "test_table",
                      "where",
                        "integer_col", 1);
  ck_assert_int_eq(h_select(conn, j_query, &j_result, NULL), H_OK);
  json_decref(j_query);
  ck_assert_ptr_ne(j_result, NULL);
  ck_assert_int_eq(json_is_array(j_result), 1);
  ck_assert_int_eq(json_array_size(j_result), 1);
  ck_assert_str_eq(json_string_value(json_object_get(json_array_get(j_result, 0), "string_col")), "value1");
  ck_assert_int_eq(json_integer_value(json_object_get(json_array_get(j_result, 0), "integer_col")), 1);
  ck_assert_double_eq(json_real_value(json_object_get(json_array_get(j_result, 0), "double_col")), 4.2);
  json_decref(j_result);
  j_query = json_pack("{sss{sssf}s{si}}",
                      "table",
                      "test_table",
                      "set",
                        "string_col", "new value1",
                        "double_col", 5.4,
                      "where",
                        "integer_col", 1);
  ck_assert_int_eq(h_update(conn, j_query, &str_query), H_OK);
  json_decref(j_query);
  ck_assert_int_eq(o_strlen(str_query), o_strlen("UPDATE test_table SET string_col='new value1', double_col=5.400000 WHERE integer_col='1'"));
  h_free(str_query);
  j_query = json_pack("{sss{si}}",
                      "table",
                      "test_table",
                      "where",
                        "integer_col", 1);
  ck_assert_int_eq(h_select(conn, j_query, &j_result, NULL), H_OK);
  ck_assert_ptr_ne(j_result, NULL);
  ck_assert_int_eq(json_array_size(j_result), 1);
  ck_assert_str_eq(json_string_value(json_object_get(json_array_get(j_result, 0), "string_col")), "new value1");
  ck_assert_int_eq(json_integer_value(json_object_get(json_array_get(j_result, 0), "integer_col")), 1);
  ck_assert_double_eq(json_real_value(json_object_get(json_array_get(j_result, 0), "double_col")), 5.4);
  json_decref(j_result);
  ck_assert_int_eq(h_delete(conn, j_query, NULL), H_OK);
  json_decref(j_query);
  ck_assert_int_eq(h_close_db(conn), H_OK);
  ck_assert_int_eq(h_clean_connection(conn), H_OK);
}
END_TEST

START_TEST(test_hoel_json_delete)
{
  struct _h_connection * conn;
  char * str_query = NULL;
  json_t * j_query = json_pack("{sss{sisss{ss}sf}}",
                               "table",
                               "test_table",
                               "values",
                                 "integer_col", 1,
                                 "string_col", "value1",
                                 "date_col",
                                   "raw",
                                   "date('now')",
                                 "double_col", 4.2),
         * j_result = NULL;
  conn = h_connect_sqlite(DEFAULT_BD_PATH);
  ck_assert_ptr_ne(conn, NULL);
  ck_assert_int_eq(h_insert(conn, j_query, NULL), H_OK);
  json_decref(j_query);
  j_query = json_pack("{sss{sisss{ss}sf}}",
                      "table",
                      "test_table",
                      "values",
                        "integer_col", 2,
                        "string_col", "value2",
                        "date_col",
                          "raw",
                          "strftime('%s','2016-06-22 00:52:56')",
                         "double_col", 5.4);
  ck_assert_int_eq(h_insert(conn, j_query, NULL), H_OK);
  json_decref(j_query);
  j_query = json_pack("{ss}",
                      "table",
                      "test_table");
  ck_assert_int_eq(h_select(conn, j_query, &j_result, NULL), H_OK);
  json_decref(j_query);
  ck_assert_ptr_ne(j_result, NULL);
  ck_assert_int_eq(json_is_array(j_result), 1);
  ck_assert_int_eq(json_array_size(j_result), 2);
  json_decref(j_result);
  j_query = json_pack("{sss{si}}",
                      "table",
                      "test_table",
                      "where",
                        "integer_col", 1);
  ck_assert_int_eq(h_delete(conn, j_query, &str_query), H_OK);
  ck_assert_int_eq(o_strlen(str_query), o_strlen("DELETE FROM test_table WHERE integer_col='1'"));
  h_free(str_query);
  json_decref(j_query);
  j_query = json_pack("{ss}",
                      "table",
                      "test_table");
  ck_assert_int_eq(h_select(conn, j_query, &j_result, NULL), H_OK);
  json_decref(j_query);
  ck_assert_int_eq(json_array_size(j_result), 1);
  ck_assert_str_eq(json_string_value(json_object_get(json_array_get(j_result, 0), "string_col")), "value2");
  ck_assert_int_eq(json_integer_value(json_object_get(json_array_get(j_result, 0), "integer_col")), 2);
  ck_assert_int_eq(json_integer_value(json_object_get(json_array_get(j_result, 0), "date_col")), 1466556776);
  ck_assert_double_eq(json_real_value(json_object_get(json_array_get(j_result, 0), "double_col")), 5.4);
  json_decref(j_result);
  j_query = json_pack("{ss}",
                      "table",
                      "test_table");
  ck_assert_int_eq(h_delete(conn, j_query, NULL), H_OK);
  ck_assert_int_eq(h_select(conn, j_query, &j_result, NULL), H_OK);
  ck_assert_int_eq(json_array_size(j_result), 0);
  json_decref(j_result);
  json_decref(j_query);
  ck_assert_int_eq(h_close_db(conn), H_OK);
  ck_assert_int_eq(h_clean_connection(conn), H_OK);
}
END_TEST

START_TEST(test_hoel_json_select)
{
  struct _h_connection * conn;
  char * str_query = NULL;
  json_t * j_query = json_pack("{sss{sisss{ss}sf}}",
                               "table",
                               "test_table",
                               "values",
                                 "integer_col", 1,
                                 "string_col", "value1",
                                 "date_col",
                                   "raw",
                                   "date('now')",
                                 "double_col", 4.2),
         * j_result = NULL;
  conn = h_connect_sqlite(DEFAULT_BD_PATH);
  ck_assert_ptr_ne(conn, NULL);
  ck_assert_int_eq(h_insert(conn, j_query, NULL), H_OK);
  json_decref(j_query);
  j_query = json_pack("{sss{sisss{ss}sf}}",
                      "table",
                      "test_table",
                      "values",
                        "integer_col", 2,
                        "string_col", "value2",
                        "date_col",
                          "raw",
                          "strftime('%s','2016-06-22 00:52:56')",
                        "double_col", 5.4);
  ck_assert_int_eq(h_insert(conn, j_query, NULL), H_OK);
  json_decref(j_query);
  
  j_query = json_pack("{ss}",
                      "table",
                      "test_table");
  ck_assert_int_eq(h_select(conn, j_query, &j_result, &str_query), H_OK);
  ck_assert_int_eq(o_strlen(str_query), o_strlen("SELECT * FROM test_table WHERE 1=1  "));
  ck_assert_int_eq(json_array_size(j_result), 2);
  ck_assert_str_eq(json_string_value(json_object_get(json_array_get(j_result, 0), "string_col")), "value1");
  ck_assert_int_eq(json_integer_value(json_object_get(json_array_get(j_result, 0), "integer_col")), 1);
  ck_assert_double_eq(json_real_value(json_object_get(json_array_get(j_result, 0), "double_col")), 4.2);
  json_decref(j_query);
  json_decref(j_result);
  h_free(str_query);
  
  j_query = json_pack("{sss{si}}",
                      "table",
                      "test_table",
                      "where",
                        "integer_col", 1);
  ck_assert_int_eq(h_select(conn, j_query, &j_result, &str_query), H_OK);
  ck_assert_int_eq(o_strlen(str_query), o_strlen("SELECT * FROM test_table WHERE integer_col='1'  "));
  ck_assert_int_eq(json_array_size(j_result), 1);
  ck_assert_str_eq(json_string_value(json_object_get(json_array_get(j_result, 0), "string_col")), "value1");
  ck_assert_int_eq(json_integer_value(json_object_get(json_array_get(j_result, 0), "integer_col")), 1);
  ck_assert_double_eq(json_real_value(json_object_get(json_array_get(j_result, 0), "double_col")), 4.2);
  json_decref(j_query);
  json_decref(j_result);
  h_free(str_query);
  
  j_query = json_pack("{sss{ss}}",
                      "table",
                      "test_table",
                      "where",
                        "string_col", "value1");
  ck_assert_int_eq(h_select(conn, j_query, &j_result, &str_query), H_OK);
  ck_assert_int_eq(o_strlen(str_query), o_strlen("SELECT * FROM test_table WHERE string_col='value1'  "));
  ck_assert_int_eq(json_array_size(j_result), 1);
  ck_assert_str_eq(json_string_value(json_object_get(json_array_get(j_result, 0), "string_col")), "value1");
  ck_assert_int_eq(json_integer_value(json_object_get(json_array_get(j_result, 0), "integer_col")), 1);
  ck_assert_double_eq(json_real_value(json_object_get(json_array_get(j_result, 0), "double_col")), 4.2);
  json_decref(j_query);
  json_decref(j_result);
  h_free(str_query);
  
  j_query = json_pack("{sss{ss}}",
                      "table",
                      "test_table",
                      "where",
                        "string_col",
                        "value'to'escape");
  ck_assert_int_eq(h_select(conn, j_query, &j_result, &str_query), H_OK);
  ck_assert_int_eq(o_strlen(str_query), o_strlen("SELECT * FROM test_table WHERE string_col='value''to''escape'  "));
  ck_assert_int_eq(json_array_size(j_result), 0);
  json_decref(j_query);
  json_decref(j_result);
  h_free(str_query);
  
  j_query = json_pack("{sss{s{ss}}}",
                      "table",
                      "test_table",
                      "where",
                        "integer_col",
                          "operator",
                          "NOT NULL");
  ck_assert_int_eq(h_select(conn, j_query, &j_result, &str_query), H_OK);
  ck_assert_int_eq(o_strlen(str_query), o_strlen("SELECT * FROM test_table WHERE integer_col IS NOT NULL  "));
  ck_assert_int_eq(json_array_size(j_result), 2);
  ck_assert_str_eq(json_string_value(json_object_get(json_array_get(j_result, 0), "string_col")), "value1");
  ck_assert_int_eq(json_integer_value(json_object_get(json_array_get(j_result, 0), "integer_col")), 1);
  ck_assert_double_eq(json_real_value(json_object_get(json_array_get(j_result, 0), "double_col")), 4.2);
  json_decref(j_query);
  json_decref(j_result);
  h_free(str_query);
  
  j_query = json_pack("{sss{s{ssss}}}",
                      "table",
                      "test_table",
                      "where",
                        "integer_col",
                          "operator",
                            "raw",
                            "value",
                            ">6");
  ck_assert_int_eq(h_select(conn, j_query, &j_result, &str_query), H_OK);
  ck_assert_int_eq(o_strlen(str_query), o_strlen("SELECT * FROM test_table WHERE integer_col >6  "));
  ck_assert_int_eq(json_array_size(j_result), 0);
  json_decref(j_query);
  json_decref(j_result);
  h_free(str_query);
  
  j_query = json_pack("{sss{s{ss}s{ssss}}}",
                      "table",
                      "test_table",
                      "where",
                        "string_col",
                          "operator",
                          "NOT NULL",
                        "integer_col",
                          "operator",
                          "raw",
                          "value",
                          ">=1");
  ck_assert_int_eq(h_select(conn, j_query, &j_result, &str_query), H_OK);
  ck_assert_int_eq(o_strlen(str_query), o_strlen("SELECT * FROM test_table WHERE string_col IS NOT NULL AND integer_col >=1  "));
  ck_assert_int_eq(json_array_size(j_result), 2);
  ck_assert_str_eq(json_string_value(json_object_get(json_array_get(j_result, 0), "string_col")), "value1");
  ck_assert_int_eq(json_integer_value(json_object_get(json_array_get(j_result, 0), "integer_col")), 1);
  ck_assert_double_eq(json_real_value(json_object_get(json_array_get(j_result, 0), "double_col")), 4.2);
  json_decref(j_query);
  json_decref(j_result);
  h_free(str_query);
  
  j_query = json_pack("{sss{so}}",
                      "table",
                      "test_table",
                      "where",
                        "integer_col",
                        json_null());
  ck_assert_int_eq(h_select(conn, j_query, &j_result, &str_query), H_OK);
  ck_assert_int_eq(o_strlen(str_query), o_strlen("SELECT * FROM test_table WHERE integer_col IS NULL  "));
  ck_assert_int_eq(json_array_size(j_result), 0);
  json_decref(j_query);
  json_decref(j_result);
  h_free(str_query);
  
  str_query = NULL;
  j_result = NULL;
  j_query = json_pack("{sss{s{sss[ii]}}}",
                      "table",
                      "test_table",
                      "where",
                        "integer_col",
                          "operator",
                          "IN",
                          "value",
                            42,
                            66);
  ck_assert_int_eq(h_select(conn, j_query, &j_result, &str_query), H_OK);
  ck_assert_int_eq(o_strlen(str_query), o_strlen("SELECT * FROM test_table WHERE integer_col IN (42,66)  "));
  ck_assert_int_eq(json_array_size(j_result), 0);
  json_decref(j_query);
  json_decref(j_result);
  h_free(str_query);
  
  j_query = json_pack("{ss}",
                      "table",
                      "test_table");
  ck_assert_int_eq(h_delete(conn, j_query, NULL), H_OK);
  ck_assert_int_eq(h_select(conn, j_query, &j_result, NULL), H_OK);
  ck_assert_int_eq(json_array_size(j_result), 0);
  json_decref(j_result);
  json_decref(j_query);
  ck_assert_int_eq(h_close_db(conn), H_OK);
  ck_assert_int_eq(h_clean_connection(conn), H_OK);
}
END_TEST

START_TEST(test_hoel_json_escape)
{
  struct _h_connection * conn;
  char * str_query = NULL, * escaped, * str_query_wip;
  json_t * j_query, * j_result = NULL;
  
  conn = h_connect_sqlite(DEFAULT_BD_PATH);
  ck_assert_ptr_ne(conn, NULL);
  ck_assert_ptr_ne(NULL, escaped = h_escape_string(conn, UNSAFE_STRING));
  j_query = json_pack("{sss{sisss{ss}}}",
                       "table",
                       "test_table",
                       "values",
                         "integer_col",
                         43,
                         "string_col",
                         UNSAFE_STRING,
                         "date_col",
                           "raw",
                           "date('now')");
  ck_assert_int_eq(h_insert(conn, j_query, &str_query), H_OK);
  json_decref(j_query);
  j_query = json_pack("{sss{si}}",
                      "table",
                      "test_table",
                      "where",
                        "integer_col",
                        43);
  ck_assert_int_eq(h_select(conn, j_query, &j_result, NULL), H_OK);
  ck_assert_ptr_ne(j_result, NULL);
  ck_assert_int_eq(json_is_array(j_result), 1);
  ck_assert_int_eq(json_array_size(j_result), 1);
  ck_assert_str_eq(json_string_value(json_object_get(json_array_get(j_result, 0), "string_col")), UNSAFE_STRING);
  ck_assert_int_eq(json_integer_value(json_object_get(json_array_get(j_result, 0), "integer_col")), 43);
  json_decref(j_result);
  ck_assert_ptr_ne(NULL, o_strstr(str_query, escaped));
  ck_assert_ptr_eq(NULL, o_strstr(str_query, UNSAFE_STRING));
  h_free(str_query);
  json_decref(j_query);
  
  j_query = json_pack("{sss{ss}}",
                      "table",
                      "test_table",
                      "where",
                        "string_col",
                        UNSAFE_STRING);
  ck_assert_int_eq(h_select(conn, j_query, &j_result, &str_query), H_OK);
  ck_assert_ptr_ne(NULL, o_strstr(str_query, escaped));
  ck_assert_ptr_eq(NULL, o_strstr(str_query, UNSAFE_STRING));
  h_free(str_query);
  ck_assert_ptr_ne(j_result, NULL);
  ck_assert_int_eq(json_is_array(j_result), 1);
  ck_assert_int_eq(json_array_size(j_result), 1);
  ck_assert_str_eq(json_string_value(json_object_get(json_array_get(j_result, 0), "string_col")), UNSAFE_STRING);
  ck_assert_int_eq(json_integer_value(json_object_get(json_array_get(j_result, 0), "integer_col")), 43);
  json_decref(j_result);
  json_decref(j_query);
  
  j_query = json_pack("{sss{sssi}s{siss}}",
                      "table",
                      "test_table",
                      "set",
                        "string_col",
                        UNSAFE_STRING " - updated",
                        "integer_col",
                        44,
                      "where",
                        "integer_col",
                        43,
                        "string_col",
                        UNSAFE_STRING);
  ck_assert_int_eq(h_update(conn, j_query, &str_query), H_OK);
  ck_assert_ptr_ne(NULL, o_strstr(str_query, escaped));
  str_query_wip = o_strstr(str_query, escaped)+o_strlen(escaped);
  ck_assert_ptr_ne(NULL, o_strstr(str_query_wip, escaped));
  ck_assert_ptr_eq(NULL, o_strstr(str_query, UNSAFE_STRING));
  h_free(str_query);
  json_decref(j_query);
  
  j_query = json_pack("{sss{siss}}",
                      "table",
                      "test_table",
                      "where",
                        "integer_col",
                        44,
                        "string_col",
                        UNSAFE_STRING " - updated");
  ck_assert_int_eq(h_select(conn, j_query, &j_result, &str_query), H_OK);
  ck_assert_ptr_ne(NULL, o_strstr(str_query, escaped));
  ck_assert_ptr_eq(NULL, o_strstr(str_query, UNSAFE_STRING));
  h_free(str_query);
  ck_assert_ptr_ne(j_result, NULL);
  ck_assert_int_eq(json_is_array(j_result), 1);
  ck_assert_int_eq(json_array_size(j_result), 1);
  ck_assert_str_eq(json_string_value(json_object_get(json_array_get(j_result, 0), "string_col")), UNSAFE_STRING " - updated");
  ck_assert_int_eq(json_integer_value(json_object_get(json_array_get(j_result, 0), "integer_col")), 44);
  json_decref(j_result);
  
  ck_assert_int_eq(h_delete(conn, j_query, NULL), H_OK);
  json_decref(j_query);

  // Test multiple escape
  j_query = json_pack("{sss[{sissss}{sissss}{sissss}]}",
                      "table",
                        "test_table",
                      "values",
                        "integer_col",
                        48,
                        "string_col",
                        UNSAFE_STRING " - 2",
                        "string_col ",
                        UNSAFE_STRING,
                        "integer_col",
                        48,
                        "string_col",
                        UNSAFE_STRING " - updated12",
                        "string_col ",
                        UNSAFE_STRING " - updated1",
                        "integer_col",
                        48,
                        "string_col",
                        UNSAFE_STRING " - updated22",
                        "string_col ",
                        UNSAFE_STRING " - updated2");
  ck_assert_int_eq(h_insert(conn, j_query, &str_query), H_OK);
  ck_assert_ptr_ne(NULL, o_strstr(str_query, escaped));
  str_query_wip = o_strstr(str_query, escaped)+o_strlen(escaped);
  ck_assert_ptr_ne(NULL, o_strstr(str_query_wip, escaped));
  str_query_wip = o_strstr(str_query, escaped)+o_strlen(escaped);
  ck_assert_ptr_ne(NULL, o_strstr(str_query_wip, escaped));
  str_query_wip = o_strstr(str_query, escaped)+o_strlen(escaped);
  ck_assert_ptr_ne(NULL, o_strstr(str_query_wip, escaped));
  str_query_wip = o_strstr(str_query, escaped)+o_strlen(escaped);
  ck_assert_ptr_ne(NULL, o_strstr(str_query_wip, escaped));
  str_query_wip = o_strstr(str_query, escaped)+o_strlen(escaped);
  ck_assert_ptr_ne(NULL, o_strstr(str_query_wip, escaped));
  str_query_wip = o_strstr(str_query, escaped)+o_strlen(escaped);
  ck_assert_ptr_ne(NULL, o_strstr(str_query_wip, escaped));
  ck_assert_ptr_eq(NULL, o_strstr(str_query, UNSAFE_STRING));
  json_decref(j_query);
  h_free(str_query);
  j_query = json_pack("{sss{si}}",
                      "table",
                        "test_table",
                      "where",
                        "integer_col",
                        48);
  ck_assert_int_eq(h_select(conn, j_query, &j_result, NULL), H_OK);
  json_decref(j_query);
  ck_assert_ptr_ne(j_result, NULL);
  ck_assert_int_eq(json_is_array(j_result), 1);
  ck_assert_int_eq(json_array_size(j_result), 3);
  ck_assert_str_eq(json_string_value(json_object_get(json_array_get(j_result, 0), "string_col")), UNSAFE_STRING " - 2");
  ck_assert_str_eq(json_string_value(json_object_get(json_array_get(j_result, 1), "string_col")), UNSAFE_STRING " - updated12");
  ck_assert_str_eq(json_string_value(json_object_get(json_array_get(j_result, 2), "string_col")), UNSAFE_STRING " - updated22");
  json_decref(j_result);
  j_query = json_pack("{sss{ssssss}}",
                      "table",
                        "test_table",
                      "where",
                        "string_col",
                        UNSAFE_STRING,
                        "string_col ",
                        UNSAFE_STRING " - updated1",
                        "string_col  ",
                        UNSAFE_STRING " - updated2");
  ck_assert_int_eq(h_select(conn, j_query, &j_result, &str_query), H_OK);
  json_decref(j_query);
  ck_assert_ptr_ne(NULL, o_strstr(str_query, escaped));
  ck_assert_ptr_ne(NULL, o_strstr(o_strstr(str_query, escaped)+o_strlen(escaped), escaped));
  str_query_wip = o_strstr(str_query, escaped)+o_strlen(escaped);
  ck_assert_ptr_ne(NULL, o_strstr(str_query_wip, escaped));
  str_query_wip = o_strstr(str_query, escaped)+o_strlen(escaped);
  ck_assert_ptr_ne(NULL, o_strstr(str_query_wip, escaped));
  ck_assert_ptr_eq(NULL, o_strstr(str_query, UNSAFE_STRING));
  ck_assert_ptr_ne(j_result, NULL);
  ck_assert_int_eq(json_is_array(j_result), 1);
  ck_assert_int_eq(json_array_size(j_result), 0);
  json_decref(j_result);
  h_free(str_query);
  j_query = json_pack("{sss{ssssss}s{sissssss}}",
                      "table",
                      "test_table",
                      "set",
                        "string_col",
                        UNSAFE_STRING " - updated",
                        "string_col ",
                        UNSAFE_STRING " - updated14",
                        "string_col  ",
                        UNSAFE_STRING " - updated24",
                      "where",
                        "integer_col",
                        48,
                        "string_col",
                        UNSAFE_STRING,
                        "string_col ",
                        UNSAFE_STRING " - updated1",
                        "string_col  ",
                        UNSAFE_STRING " - updated2");
  ck_assert_int_eq(h_update(conn, j_query, &str_query), H_OK);
  ck_assert_ptr_ne(NULL, o_strstr(str_query, escaped));
  str_query_wip = o_strstr(str_query, escaped)+o_strlen(escaped);
  ck_assert_ptr_ne(NULL, o_strstr(str_query_wip, escaped));
  str_query_wip = o_strstr(str_query, escaped)+o_strlen(escaped);
  ck_assert_ptr_ne(NULL, o_strstr(str_query_wip, escaped));
  str_query_wip = o_strstr(str_query, escaped)+o_strlen(escaped);
  ck_assert_ptr_ne(NULL, o_strstr(str_query_wip, escaped));
  str_query_wip = o_strstr(str_query, escaped)+o_strlen(escaped);
  ck_assert_ptr_ne(NULL, o_strstr(str_query_wip, escaped));
  str_query_wip = o_strstr(str_query, escaped)+o_strlen(escaped);
  ck_assert_ptr_ne(NULL, o_strstr(str_query_wip, escaped));
  ck_assert_ptr_eq(NULL, o_strstr(str_query, UNSAFE_STRING));
  h_free(str_query);
  json_decref(j_query);

  j_query = json_pack("{sss{si}}",
                      "table",
                      "test_table",
                      "where",
                        "integer_col",
                        48);
  ck_assert_int_eq(h_delete(conn, j_query, NULL), H_OK);
  json_decref(j_query);
  
  h_free(escaped);
  ck_assert_int_eq(h_close_db(conn), H_OK);
  ck_assert_int_eq(h_clean_connection(conn), H_OK);
}
END_TEST

START_TEST(test_hoel_json_generate_where_clause)
{
  struct _h_connection * conn;
  char * where_clause = NULL;
  json_t * j_query = json_pack("{sss{sisss{ss}}}",
                               "table",
                               "test_table",
                               "values",
                                 "integer_col",
                                 55,
                                 "string_col",
                                 "value1",
                                 "date_col",
                                   "raw",
                                   "date('now')"), * j_result = NULL, * j_string = json_string(UNSAFE_STRING), * j_integer = json_integer(22), * j_real = json_real(9.9);
  conn = h_connect_sqlite(DEFAULT_BD_PATH);
  ck_assert_ptr_ne(conn, NULL);
  ck_assert_int_eq(h_insert(conn, j_query, NULL), H_OK);
  json_decref(j_query);
  
  where_clause = h_build_where_clause(conn, "integer_col = %d AND string_col = %s", (json_int_t)55, "value1");
  j_query = json_pack("{sss{s{ssss}}}",
                      "table",
                      "test_table",
                      "where",
                        "",
                          "operator",
                          "raw",
                          "value",
                          where_clause);
  ck_assert_int_eq(h_select(conn, j_query, &j_result, NULL), H_OK);
  ck_assert_ptr_ne(j_result, NULL);
  ck_assert_int_eq(json_is_array(j_result), 1);
  ck_assert_int_eq(json_array_size(j_result), 1);
  json_decref(j_query);
  json_decref(j_result);
  h_free(where_clause);

  where_clause = h_build_where_clause(conn, "0.0 = %f AND '%%lol' = '%%lol' AND string_col = %s", (double)0.0, UNSAFE_STRING);
  j_query = json_pack("{sss{s{ssss}}}",
                      "table",
                      "test_table",
                      "where",
                        "",
                          "operator",
                          "raw",
                          "value",
                          where_clause);
  ck_assert_int_eq(h_select(conn, j_query, &j_result, NULL), H_OK);
  ck_assert_ptr_ne(j_result, NULL);
  ck_assert_int_eq(json_is_array(j_result), 1);
  ck_assert_int_eq(json_array_size(j_result), 0);
  json_decref(j_query);
  json_decref(j_result);
  h_free(where_clause);

  where_clause = h_build_where_clause(conn, "0.0 = %f AND '%%lol' = '%%lol' AND string_col = '%S'", (double)0.0, UNSAFE_STRING);
  j_query = json_pack("{sss{s{ssss}}}",
                      "table",
                      "test_table",
                      "where",
                        "",
                          "operator",
                          "raw",
                          "value",
                          where_clause);
  ck_assert_int_eq(h_select(conn, j_query, &j_result, NULL), H_OK);
  ck_assert_ptr_ne(j_result, NULL);
  ck_assert_int_eq(json_is_array(j_result), 1);
  ck_assert_int_eq(json_array_size(j_result), 0);
  json_decref(j_query);
  json_decref(j_result);
  h_free(where_clause);

  where_clause = h_build_where_clause(conn, "string_col = '%S' OR string_col = '%S' OR string_col = '%S'", UNSAFE_STRING, UNSAFE_STRING " - updated1", UNSAFE_STRING " - updated2");
  j_query = json_pack("{sss{s{ssss}}}",
                      "table",
                      "test_table",
                      "where",
                        "",
                          "operator",
                          "raw",
                          "value",
                          where_clause);
  ck_assert_int_eq(h_select(conn, j_query, &j_result, NULL), H_OK);
  ck_assert_ptr_ne(j_result, NULL);
  ck_assert_int_eq(json_is_array(j_result), 1);
  ck_assert_int_eq(json_array_size(j_result), 0);
  json_decref(j_query);
  json_decref(j_result);
  h_free(where_clause);

  where_clause = h_build_where_clause(conn, "string_col = '%S' OR string_col = %s OR string_col = %c OR string_col = '%C'", UNSAFE_STRING, UNSAFE_STRING " - updated1", "value1", "value2");
  j_query = json_pack("{sss{s{ssss}}}",
                      "table",
                      "test_table",
                      "where",
                        "",
                          "operator",
                          "raw",
                          "value",
                          where_clause);
  ck_assert_int_eq(h_select(conn, j_query, &j_result, NULL), H_OK);
  ck_assert_ptr_ne(j_result, NULL);
  ck_assert_int_eq(json_is_array(j_result), 1);
  ck_assert_int_eq(json_array_size(j_result), 1);
  json_decref(j_query);
  json_decref(j_result);
  h_free(where_clause);

  where_clause = h_build_where_clause(conn, "0.0 = %f AND '%%lol' = '%%lol' AND string_col = %c", (double)0.0, "value1");
  j_query = json_pack("{sss{s{ssss}}}",
                      "table",
                      "test_table",
                      "where",
                        "",
                          "operator",
                          "raw",
                          "value",
                          where_clause);
  ck_assert_int_eq(h_select(conn, j_query, &j_result, NULL), H_OK);
  ck_assert_ptr_ne(j_result, NULL);
  ck_assert_int_eq(json_is_array(j_result), 1);
  ck_assert_int_eq(json_array_size(j_result), 1);
  json_decref(j_query);
  json_decref(j_result);
  h_free(where_clause);

  where_clause = h_build_where_clause(conn, "0.0 = %f AND '%%lol' = '%%lol' AND string_col = '%C'", (double)0.0, "value1");
  j_query = json_pack("{sss{s{ssss}}}",
                      "table",
                      "test_table",
                      "where",
                        "",
                          "operator",
                          "raw",
                          "value",
                          where_clause);
  ck_assert_int_eq(h_select(conn, j_query, &j_result, NULL), H_OK);
  ck_assert_ptr_ne(j_result, NULL);
  ck_assert_int_eq(json_is_array(j_result), 1);
  ck_assert_int_eq(json_array_size(j_result), 1);
  json_decref(j_query);
  json_decref(j_result);
  h_free(where_clause);

  where_clause = h_build_where_clause(conn, "0.0 = %f AND '%%lol' = '%%lol' AND string_col = %c", (double)0.0, UNSAFE_STRING);
  j_query = json_pack("{sss{s{ssss}}}",
                      "table",
                      "test_table",
                      "where",
                        "",
                          "operator",
                          "raw",
                          "value",
                          where_clause);
  j_result = NULL;
  ck_assert_int_eq(h_select(conn, j_query, &j_result, NULL), H_ERROR_QUERY);
  ck_assert_ptr_eq(j_result, NULL);
  json_decref(j_query);
  h_free(where_clause);

  j_query = json_pack("{sss{si}}",
                      "table",
                      "test_table",
                      "where",
                        "integer_col",
                        55);
  ck_assert_int_eq(h_delete(conn, j_query, NULL), H_OK);
  json_decref(j_query);

  where_clause = h_build_where_clause(conn, "0.0 = %j AND integer_col = %j AND string_col = %j AND string_col = %s", j_real, j_integer, j_string, UNSAFE_STRING);
  j_query = json_pack("{sss{s{ssss}}}",
                      "table",
                      "test_table",
                      "where",
                        "",
                          "operator",
                          "raw",
                          "value",
                          where_clause);
  ck_assert_int_eq(h_select(conn, j_query, &j_result, NULL), H_OK);
  ck_assert_ptr_ne(j_result, NULL);
  ck_assert_int_eq(json_is_array(j_result), 1);
  ck_assert_int_eq(json_array_size(j_result), 0);
  json_decref(j_query);
  json_decref(j_result);
  h_free(where_clause);

  where_clause = h_build_where_clause(conn, WHERE_CLAUSE_NO_FORMAT);
  ck_assert_str_eq(where_clause, WHERE_CLAUSE_NO_FORMAT);
  h_free(where_clause);
  
  ck_assert_ptr_eq(NULL, h_build_where_clause(NULL, "0.0 = %f AND '%%lol' = '%%lol' AND string_col = '%S'", (double)0.0, UNSAFE_STRING));
  ck_assert_ptr_eq(NULL, h_build_where_clause(conn, "error = %j", json_null()));
  ck_assert_ptr_eq(NULL, h_build_where_clause(conn, NULL, (double)0.0, UNSAFE_STRING));
  ck_assert_ptr_eq(NULL, h_build_where_clause(conn, "", (double)0.0, UNSAFE_STRING));
  ck_assert_ptr_eq(NULL, h_build_where_clause(conn, "this is an error %", (double)0.0, UNSAFE_STRING));
  ck_assert_ptr_eq(NULL, h_build_where_clause(conn, "this is another error %n to test", (double)0.0, UNSAFE_STRING));
  
  json_decref(j_real);
  json_decref(j_integer);
  json_decref(j_string);
  ck_assert_int_eq(h_close_db(conn), H_OK);
  ck_assert_int_eq(h_clean_connection(conn), H_OK);
}
END_TEST

static Suite *hoel_suite(void)
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("Hoel core tests");
	tc_core = tcase_create("test_hoel_core");
	tcase_add_test(tc_core, test_hoel_init);
	tcase_add_test(tc_core, test_hoel_escape_string);
	tcase_add_test(tc_core, test_hoel_escape_string_with_quotes);
	tcase_add_test(tc_core, test_hoel_insert);
	tcase_add_test(tc_core, test_hoel_update);
	tcase_add_test(tc_core, test_hoel_delete);
	tcase_add_test(tc_core, test_hoel_json_insert);
	tcase_add_test(tc_core, test_hoel_json_update);
	tcase_add_test(tc_core, test_hoel_json_delete);
	tcase_add_test(tc_core, test_hoel_json_select);
	tcase_add_test(tc_core, test_hoel_json_escape);
	tcase_add_test(tc_core, test_hoel_json_generate_where_clause);
	tcase_set_timeout(tc_core, 30);
	suite_add_tcase(s, tc_core);

	return s;
}

int main(int argc, char *argv[])
{
  int number_failed;
  Suite *s;
  SRunner *sr;
  //y_init_logs("Hoel", Y_LOG_MODE_CONSOLE, Y_LOG_LEVEL_DEBUG, NULL, "Starting Hoel core tests");

  s = hoel_suite();
  sr = srunner_create(s);

  srunner_run_all(sr, CK_VERBOSE);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  
  //y_close_logs();
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
