/* Public domain, no copyright. Use at your own risk. */
/* This program tests the behavior is correct in all supported backends */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <jansson.h>

#include <check.h>

#include "hoel.h"

/**************************************************/
/* Set the database backend you want to test here */
/**************************************************/
#define SQLITE
//#define MARIADB
//#define PGSQL

#ifdef SQLITE
  #define SQLITE_BD_PATH "/tmp/test.db"
  #define NOW "date('now')"
  #define DATE_FIXED "strftime('%s','2016-06-22 00:52:56')"
  #define SAFE "unsafe '' value\"!"
  #define SAFE_QUOTE "'unsafe '' value\"!'"
#endif

#ifdef MARIADB
  #define MARIADB_HOST "localhost"
  #define MARIADB_USER "test"
  #define MARIADB_PASSWD "test"
  #define MARIADB_DB "test"
  #define MARIADB_PORT 0
  #define NOW "NOW()"
  #define DATE_FIXED "STR_TO_DATE('2016-06-22 00:52:56','%Y-%m-%d %H:%i:%s')"
  #define SAFE "unsafe \\' value\\\"!"
  #define SAFE_QUOTE "'unsafe \\' value\\\"!'"
#endif

#ifdef PGSQL
  #define PGSQL_CONNINFO "host=localhost dbname=test user=test password=test"
  #define NOW "NOW()"
  #define DATE_FIXED "to_date('2016-06-22 00:52:56','YYYY-MM-DD HH24:MI:SS')"
  #define SAFE "unsafe '' value\"!"
  #define SAFE_QUOTE "'unsafe '' value\"!'"
#endif


#define UNSAFE_STRING "un'safe' (\"string\")#!/$%*];"

#define SELECT_DATA_1 "SELECT integer_col, string_col, date_col FROM test_table WHERE integer_col = 1"
#define SELECT_DATA_2 "SELECT integer_col, string_col, date_col FROM test_table WHERE integer_col = 2"
#define SELECT_DATA_ERROR "SELECT integer_col, string_col, date_col FROM test_table WHERE integer_col = 'error'"
#define SELECT_DATA_ALL "SELECT * FROM test_table"

#define INSERT_DATA_1 "INSERT INTO test_table (integer_col, string_col, date_col) VALUES (1, 'value1', "NOW")"
#define INSERT_DATA_2 "INSERT INTO test_table (integer_col, string_col, date_col) VALUES (2, 'value2', "DATE_FIXED")"
#define INSERT_DATA_ERROR "INSERT INTO test_table (integer_col, string_col, date_col) VALUES ('error', 'value error', "NOW")"

#define DELETE_DATA_1 "DELETE FROM test_table WHERE integer_col = 1"
#define DELETE_DATA_2 "DELETE FROM test_table WHERE integer_col = 2"
#define DELETE_DATA_ERROR "DELETE FROM test_table WHERE wrong_table = 1"
#define DELETE_DATA_ALL "DELETE FROM test_table"

#define UPDATE_DATA_1 "UPDATE test_table SET string_col='new value1' WHERE integer_col = 1"

START_TEST(test_hoel_escape_string)
{
  char * escaped;
  json_t * j_query, * j_result;
  int res;
  
  struct _h_connection * conn = NULL;
#ifdef SQLITE
  // Sqlite3
  conn = h_connect_sqlite(SQLITE_BD_PATH);
#endif
  
#ifdef MARIADB
  // Mysql
  conn = h_connect_mariadb(MARIADB_HOST, MARIADB_USER, MARIADB_PASSWD, MARIADB_DB, MARIADB_PORT, NULL);
#endif
  
#ifdef PGSQL
  // PostgreSQL
  conn = h_connect_pgsql(PGSQL_CONNINFO);
#endif
  
  escaped = h_escape_string(conn, "value");
  ck_assert_str_eq(escaped, "value");
  h_free(escaped);
  escaped = h_escape_string(conn, "unsafe ' value\"!");
  ck_assert_str_eq(escaped, SAFE);
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
  h_close_db(conn);
  h_clean_connection(conn);
}
END_TEST

START_TEST(test_hoel_escape_string_with_quotes)
{
  char * escaped;
  json_t * j_query, * j_result;
  int res;
  
  struct _h_connection * conn = NULL;
#ifdef SQLITE
  // Sqlite3
  conn = h_connect_sqlite(SQLITE_BD_PATH);
#endif
  
#ifdef MARIADB
  // Mysql
  conn = h_connect_mariadb(MARIADB_HOST, MARIADB_USER, MARIADB_PASSWD, MARIADB_DB, MARIADB_PORT, NULL);
#endif
  
#ifdef PGSQL
  // PostgreSQL
  conn = h_connect_pgsql(PGSQL_CONNINFO);
#endif
  
  escaped = h_escape_string_with_quotes(conn, "value");
  ck_assert_str_eq(escaped, "'value'");
  h_free(escaped);
  escaped = h_escape_string_with_quotes(conn, "unsafe ' value\"!");
  ck_assert_str_eq(escaped, SAFE_QUOTE);
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
  h_close_db(conn);
  h_clean_connection(conn);
}
END_TEST

START_TEST(test_hoel_insert)
{
  struct _h_result result;
  struct _h_data * last_id;
  
  struct _h_connection * conn = NULL;
#ifdef SQLITE
  // Sqlite3
  conn = h_connect_sqlite(SQLITE_BD_PATH);
#endif
  
#ifdef MARIADB
  // Mysql
  conn = h_connect_mariadb(MARIADB_HOST, MARIADB_USER, MARIADB_PASSWD, MARIADB_DB, MARIADB_PORT, NULL);
#endif
  
#ifdef PGSQL
  // PostgreSQL
  conn = h_connect_pgsql(PGSQL_CONNINFO);
#endif
  
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
  ck_assert_int_eq(result.nb_columns, 3);
  ck_assert_int_eq(result.data[0][0].type, HOEL_COL_TYPE_INT);
  ck_assert_int_eq(((struct _h_type_int *)result.data[0][0].t_data)->value, 1);
  ck_assert_int_eq(result.data[0][1].type, HOEL_COL_TYPE_TEXT);
  ck_assert_str_eq(((struct _h_type_text *)result.data[0][1].t_data)->value, "value1");
  ck_assert_int_eq(h_clean_result(&result), H_OK);
#ifdef PGSQL
  ck_assert_int_eq(h_query_select(conn, SELECT_DATA_ERROR, &result), H_ERROR_QUERY);
#else
  ck_assert_int_eq(h_query_select(conn, SELECT_DATA_ERROR, &result), H_OK);
  ck_assert_int_eq(result.nb_rows, 0);
  ck_assert_int_eq(result.nb_columns, 3);
  ck_assert_int_eq(h_clean_result(&result), H_OK);
#endif
  ck_assert_int_eq(h_query_delete(conn, DELETE_DATA_1), H_OK);
  h_close_db(conn);
  h_clean_connection(conn);
}
END_TEST

START_TEST(test_hoel_update)
{
  struct _h_result result;
  
  struct _h_connection * conn = NULL;
#ifdef SQLITE
  // Sqlite3
  conn = h_connect_sqlite(SQLITE_BD_PATH);
#endif
  
#ifdef MARIADB
  // Mysql
  conn = h_connect_mariadb(MARIADB_HOST, MARIADB_USER, MARIADB_PASSWD, MARIADB_DB, MARIADB_PORT, NULL);
#endif
  
#ifdef PGSQL
  // PostgreSQL
  conn = h_connect_pgsql(PGSQL_CONNINFO);
#endif
  
  ck_assert_int_eq(h_query_insert(conn, INSERT_DATA_1), H_OK);
  ck_assert_int_eq(h_query_insert(conn, NULL), H_ERROR_PARAMS);
  ck_assert_int_eq(h_query_select(conn, SELECT_DATA_1, &result), H_OK);
  ck_assert_int_eq(result.nb_rows, 1);
  ck_assert_int_eq(result.nb_columns, 3);
  ck_assert_int_eq(((struct _h_type_int *)result.data[0][0].t_data)->value, 1);
  ck_assert_str_eq(((struct _h_type_text *)result.data[0][1].t_data)->value, "value1");
  ck_assert_int_eq(h_clean_result(&result), H_OK);
  ck_assert_int_eq(h_query_update(conn, UPDATE_DATA_1), H_OK);
  ck_assert_int_eq(h_query_select(conn, SELECT_DATA_1, &result), H_OK);
  ck_assert_int_eq(result.nb_rows, 1);
  ck_assert_int_eq(result.nb_columns, 3);
  ck_assert_int_eq(((struct _h_type_int *)result.data[0][0].t_data)->value, 1);
  ck_assert_str_eq(((struct _h_type_text *)result.data[0][1].t_data)->value, "new value1");
  ck_assert_int_eq(h_clean_result(&result), H_OK);
  ck_assert_int_eq(h_query_delete(conn, DELETE_DATA_1), H_OK);
  h_close_db(conn);
  h_clean_connection(conn);
}
END_TEST

START_TEST(test_hoel_delete)
{
  struct _h_result result;
  
  struct _h_connection * conn = NULL;
#ifdef SQLITE
  // Sqlite3
  conn = h_connect_sqlite(SQLITE_BD_PATH);
#endif
  
#ifdef MARIADB
  // Mysql
  conn = h_connect_mariadb(MARIADB_HOST, MARIADB_USER, MARIADB_PASSWD, MARIADB_DB, MARIADB_PORT, NULL);
#endif
  
#ifdef PGSQL
  // PostgreSQL
  conn = h_connect_pgsql(PGSQL_CONNINFO);
#endif
  
  ck_assert_int_eq(h_query_insert(conn, INSERT_DATA_1), H_OK);
  ck_assert_int_eq(h_query_insert(conn, INSERT_DATA_2), H_OK);
  ck_assert_int_eq(h_query_select(conn, SELECT_DATA_1, &result), H_OK);
  ck_assert_int_eq(result.nb_rows, 1);
  ck_assert_int_eq(result.nb_columns, 3);
  ck_assert_int_eq(result.data[0][0].type, HOEL_COL_TYPE_INT);
  ck_assert_int_eq(((struct _h_type_int *)result.data[0][0].t_data)->value, 1);
  ck_assert_int_eq(result.data[0][1].type, HOEL_COL_TYPE_TEXT);
  ck_assert_str_eq(((struct _h_type_text *)result.data[0][1].t_data)->value, "value1");
  ck_assert_int_eq(h_clean_result(&result), H_OK);
  ck_assert_int_eq(h_query_select(conn, SELECT_DATA_2, &result), H_OK);
  ck_assert_int_eq(result.nb_rows, 1);
  ck_assert_int_eq(result.nb_columns, 3);
  ck_assert_int_eq(result.data[0][0].type, HOEL_COL_TYPE_INT);
  ck_assert_int_eq(((struct _h_type_int *)result.data[0][0].t_data)->value, 2);
  ck_assert_int_eq(result.data[0][1].type, HOEL_COL_TYPE_TEXT);
  ck_assert_str_eq(((struct _h_type_text *)result.data[0][1].t_data)->value, "value2");
  ck_assert_int_eq(h_clean_result(&result), H_OK);
  ck_assert_int_eq(h_query_delete(conn, DELETE_DATA_1), H_OK);
  ck_assert_int_eq(h_query_select(conn, SELECT_DATA_ALL, &result), H_OK);
  ck_assert_int_eq(result.nb_rows, 1);
  ck_assert_int_eq(h_clean_result(&result), H_OK);
  ck_assert_int_eq(h_query_delete(conn, DELETE_DATA_2), H_OK);
  ck_assert_int_eq(h_query_select(conn, SELECT_DATA_ALL, &result), H_OK);
  ck_assert_int_eq(result.nb_rows, 0);
  ck_assert_int_eq(h_clean_result(&result), H_OK);
  h_close_db(conn);
  h_clean_connection(conn);
}
END_TEST

START_TEST(test_hoel_json_insert)
{
  
  struct _h_connection * conn = NULL;
#ifdef SQLITE
  // Sqlite3
  conn = h_connect_sqlite(SQLITE_BD_PATH);
#endif
  
#ifdef MARIADB
  // Mysql
  conn = h_connect_mariadb(MARIADB_HOST, MARIADB_USER, MARIADB_PASSWD, MARIADB_DB, MARIADB_PORT, NULL);
#endif
  
#ifdef PGSQL
  // PostgreSQL
  conn = h_connect_pgsql(PGSQL_CONNINFO);
#endif
  
  char * str_query = NULL;
  json_t * j_query = json_pack("{sss{sisss{ss}}}",
                               "table",
                               "test_table",
                               "values",
                                 "integer_col",
                                 1,
                                 "string_col",
                                 "value1",
                                 "date_col",
                                   "raw",
                                   NOW), * j_result = NULL;
  ck_assert_int_eq(h_insert(conn, j_query, &str_query), H_OK);
  json_decref(j_query);
  j_query = json_pack("{sss{si}}",
                      "table",
                      "test_table",
                      "where",
                        "integer_col",
                        1);
  ck_assert_int_eq(h_select(conn, j_query, &j_result, NULL), H_OK);
  ck_assert_ptr_ne(j_result, NULL);
  ck_assert_int_eq(json_is_array(j_result), 1);
  ck_assert_int_eq(json_array_size(j_result), 1);
  ck_assert_str_eq(json_string_value(json_object_get(json_array_get(j_result, 0), "string_col")), "value1");
  ck_assert_int_eq(json_integer_value(json_object_get(json_array_get(j_result, 0), "integer_col")), 1);
  json_decref(j_result);
  ck_assert_int_eq(o_strlen(str_query), o_strlen("INSERT INTO test_table (integer_col,string_col,date_col) VALUES (1,'value1',"NOW")"));
  h_free(str_query);
  ck_assert_int_eq(h_delete(conn, j_query, NULL), H_OK);
  json_decref(j_query);
  h_close_db(conn);
  h_clean_connection(conn);
}
END_TEST

START_TEST(test_hoel_json_update)
{
  
  struct _h_connection * conn = NULL;
#ifdef SQLITE
  // Sqlite3
  conn = h_connect_sqlite(SQLITE_BD_PATH);
#endif
  
#ifdef MARIADB
  // Mysql
  conn = h_connect_mariadb(MARIADB_HOST, MARIADB_USER, MARIADB_PASSWD, MARIADB_DB, MARIADB_PORT, NULL);
#endif
  
#ifdef PGSQL
  // PostgreSQL
  conn = h_connect_pgsql(PGSQL_CONNINFO);
#endif
  
  char * str_query = NULL;
  json_t * j_query = json_pack("{sss{sisss{ss}}}",
                               "table",
                               "test_table",
                               "values",
                                 "integer_col",
                                 1,
                                 "string_col",
                                 "value1",
                                 "date_col",
                                   "raw",
                                   NOW),
         * j_result = NULL;
  ck_assert_int_eq(h_insert(conn, j_query, NULL), H_OK);
  json_decref(j_query);
  j_query = json_pack("{sss{si}}",
                      "table",
                      "test_table",
                      "where",
                        "integer_col",
                        1);
  ck_assert_int_eq(h_select(conn, j_query, &j_result, NULL), H_OK);
  json_decref(j_query);
  ck_assert_ptr_ne(j_result, NULL);
  ck_assert_int_eq(json_is_array(j_result), 1);
  ck_assert_int_eq(json_array_size(j_result), 1);
  ck_assert_str_eq(json_string_value(json_object_get(json_array_get(j_result, 0), "string_col")), "value1");
  ck_assert_int_eq(json_integer_value(json_object_get(json_array_get(j_result, 0), "integer_col")), 1);
  json_decref(j_result);
  j_query = json_pack("{sss{ss}s{si}}",
                      "table",
                      "test_table",
                      "set",
                        "string_col",
                        "new value1",
                      "where",
                        "integer_col",
                        1);
  ck_assert_int_eq(h_update(conn, j_query, &str_query), H_OK);
  json_decref(j_query);
  ck_assert_int_eq(o_strlen(str_query), o_strlen("UPDATE test_table SET string_col='new value1' WHERE integer_col='1'"));
  h_free(str_query);
  j_query = json_pack("{sss{si}}",
                      "table",
                      "test_table",
                      "where",
                        "integer_col",
                        1);
  ck_assert_int_eq(h_select(conn, j_query, &j_result, NULL), H_OK);
  ck_assert_ptr_ne(j_result, NULL);
  ck_assert_int_eq(json_array_size(j_result), 1);
  ck_assert_str_eq(json_string_value(json_object_get(json_array_get(j_result, 0), "string_col")), "new value1");
  ck_assert_int_eq(json_integer_value(json_object_get(json_array_get(j_result, 0), "integer_col")), 1);
  json_decref(j_result);
  ck_assert_int_eq(h_delete(conn, j_query, NULL), H_OK);
  json_decref(j_query);
  h_close_db(conn);
  h_clean_connection(conn);
}
END_TEST

START_TEST(test_hoel_json_delete)
{
  
  struct _h_connection * conn = NULL;
#ifdef SQLITE
  // Sqlite3
  conn = h_connect_sqlite(SQLITE_BD_PATH);
#endif
  
#ifdef MARIADB
  // Mysql
  conn = h_connect_mariadb(MARIADB_HOST, MARIADB_USER, MARIADB_PASSWD, MARIADB_DB, MARIADB_PORT, NULL);
#endif
  
#ifdef PGSQL
  // PostgreSQL
  conn = h_connect_pgsql(PGSQL_CONNINFO);
#endif
  
  char * str_query = NULL;
  json_t * j_query = json_pack("{sss{sisss{ss}}}",
                               "table",
                               "test_table",
                               "values",
                                 "integer_col",
                                 1,
                                 "string_col",
                                 "value1",
                                 "date_col",
                                   "raw",
                                   NOW),
         * j_result = NULL;
  ck_assert_int_eq(h_insert(conn, j_query, NULL), H_OK);
  json_decref(j_query);
  j_query = json_pack("{sss{sisss{ss}}}",
                      "table",
                      "test_table",
                      "values",
                        "integer_col",
                        2,
                        "string_col",
                        "value2",
                        "date_col",
                          "raw",
                          DATE_FIXED);
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
                        "integer_col",
                        1);
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
#ifdef SQLITE
  ck_assert_int_eq(json_integer_value(json_object_get(json_array_get(j_result, 0), "date_col")), 1466556776);
#endif
#ifdef MARIADB
  ck_assert_str_eq(json_string_value(json_object_get(json_array_get(j_result, 0), "date_col")), "2016-06-22T00:52:56");
#endif
  json_decref(j_result);
  j_query = json_pack("{ss}",
                      "table",
                      "test_table");
  ck_assert_int_eq(h_delete(conn, j_query, NULL), H_OK);
  ck_assert_int_eq(h_select(conn, j_query, &j_result, NULL), H_OK);
  ck_assert_int_eq(json_array_size(j_result), 0);
  json_decref(j_result);
  json_decref(j_query);
  h_close_db(conn);
  h_clean_connection(conn);
}
END_TEST

START_TEST(test_hoel_json_select)
{
  
  struct _h_connection * conn = NULL;
#ifdef SQLITE
  // Sqlite3
  conn = h_connect_sqlite(SQLITE_BD_PATH);
#endif
  
#ifdef MARIADB
  // Mysql
  conn = h_connect_mariadb(MARIADB_HOST, MARIADB_USER, MARIADB_PASSWD, MARIADB_DB, MARIADB_PORT, NULL);
#endif
  
#ifdef PGSQL
  // PostgreSQL
  conn = h_connect_pgsql(PGSQL_CONNINFO);
#endif
  
  char * str_query = NULL;
  json_t * j_query = json_pack("{sss{sisss{ss}}}",
                               "table",
                               "test_table",
                               "values",
                                 "integer_col",
                                 1,
                                 "string_col",
                                 "value1",
                                 "date_col",
                                   "raw",
                                   NOW),
         * j_result = NULL;
  ck_assert_ptr_ne(conn, NULL);
  ck_assert_int_eq(h_insert(conn, j_query, NULL), H_OK);
  json_decref(j_query);
  j_query = json_pack("{sss{sisss{ss}}}",
                      "table",
                      "test_table",
                      "values",
                        "integer_col",
                        2,
                        "string_col",
                        "value2",
                        "date_col",
                          "raw",
                          DATE_FIXED);
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
  json_decref(j_query);
  json_decref(j_result);
  h_free(str_query);
  
  j_query = json_pack("{sss{si}}",
                      "table",
                      "test_table",
                      "where",
                        "integer_col",
                        1);
  ck_assert_int_eq(h_select(conn, j_query, &j_result, &str_query), H_OK);
  ck_assert_int_eq(o_strlen(str_query), o_strlen("SELECT * FROM test_table WHERE integer_col='1'  "));
  ck_assert_int_eq(json_array_size(j_result), 1);
  ck_assert_str_eq(json_string_value(json_object_get(json_array_get(j_result, 0), "string_col")), "value1");
  ck_assert_int_eq(json_integer_value(json_object_get(json_array_get(j_result, 0), "integer_col")), 1);
  json_decref(j_query);
  json_decref(j_result);
  h_free(str_query);
  
  j_query = json_pack("{sss{ss}}",
                      "table",
                      "test_table",
                      "where",
                        "string_col",
                        "value1");
  ck_assert_int_eq(h_select(conn, j_query, &j_result, &str_query), H_OK);
  ck_assert_int_eq(o_strlen(str_query), o_strlen("SELECT * FROM test_table WHERE string_col='value1'  "));
  ck_assert_int_eq(json_array_size(j_result), 1);
  ck_assert_str_eq(json_string_value(json_object_get(json_array_get(j_result, 0), "string_col")), "value1");
  ck_assert_int_eq(json_integer_value(json_object_get(json_array_get(j_result, 0), "integer_col")), 1);
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
  json_decref(j_result);
  json_decref(j_query);
  
  j_query = json_pack("{sss[{sisss{ss}}{sisos{ss}}]}",
                       "table",
                       "test_table",
                       "values",
                         "integer_col",
                         1,
                         "string_col",
                         "",
                         "date_col",
                           "raw",
                           NOW,
                         "integer_col",
                         1,
                         "string_col",
                         json_null(),
                         "date_col",
                           "raw",
                           NOW);
  ck_assert_int_eq(h_insert(conn, j_query, NULL), H_OK);
  json_decref(j_query);
  j_query = json_pack("{ss}",
                      "table",
                      "test_table");
  ck_assert_int_eq(h_select(conn, j_query, &j_result, NULL), H_OK);
  ck_assert_int_eq(json_array_size(j_result), 2);
  ck_assert_str_eq(json_string_value(json_object_get(json_array_get(j_result, 0), "string_col")), "");
  ck_assert_ptr_eq(json_object_get(json_array_get(j_result, 1), "string_col"), json_null());

  json_decref(j_query);
  json_decref(j_result);

  j_query = json_pack("{ss}",
                      "table",
                      "test_table");
  ck_assert_int_eq(h_delete(conn, j_query, NULL), H_OK);
  ck_assert_int_eq(h_select(conn, j_query, &j_result, NULL), H_OK);
  json_decref(j_result);
  json_decref(j_query);
  h_close_db(conn);
  h_clean_connection(conn);
}
END_TEST

static Suite *hoel_suite(void)
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("Hoel multi-backend tests");
	tc_core = tcase_create("test_hoel_multi_backend");
	tcase_add_test(tc_core, test_hoel_escape_string);
	tcase_add_test(tc_core, test_hoel_escape_string_with_quotes);
	tcase_add_test(tc_core, test_hoel_insert);
	tcase_add_test(tc_core, test_hoel_update);
	tcase_add_test(tc_core, test_hoel_delete);
	tcase_add_test(tc_core, test_hoel_json_insert);
	tcase_add_test(tc_core, test_hoel_json_update);
	tcase_add_test(tc_core, test_hoel_json_delete);
	tcase_add_test(tc_core, test_hoel_json_select);
	tcase_set_timeout(tc_core, 30);
	suite_add_tcase(s, tc_core);

	return s;
}

int main(void)
{
  int number_failed;
  Suite *s;
  SRunner *sr;
  y_init_logs("Hoel", Y_LOG_MODE_CONSOLE, Y_LOG_LEVEL_DEBUG, NULL, "Starting Hoel multi-backend tests");
  
  s = hoel_suite();
  sr = srunner_create(s);

  srunner_run_all(sr, CK_VERBOSE);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  
  y_close_logs();
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
