#include <stdio.h>
#include <stdlib.h>
#include <jansson.h>
#define _HOEL_MARIADB
#include "../src/hoel.h"

void unit_tests(struct _h_connection * conn) {
  json_t * j_result, * j_where, * j_array, * j_set, * j_data, * j_query;
  char * table = "other_test", * dump;
  int res;
  
  j_where = json_object();
  json_object_set_new(j_where, "age", json_pack("{sssi}", "operator", ">", "value", 46));
  json_object_set_new(j_where, "name", json_pack("{ssss}", "operator", "LIKE", "value", "Hodor%"));

  j_query = json_object();
  json_object_set_new(j_query, "table", json_string(table));
  json_object_set_new(j_query, "where", j_where);
  
  res = h_select(conn, j_query, &j_result, NULL);
  if (res == H_OK) {
    dump = json_dumps(j_result, JSON_INDENT(2));
    printf("json select result is\n%s\n", dump);
    json_decref(j_result);
    free(dump);
  } else {
    printf("Error executing select query: %d\n", res);
  }
  json_decref(j_query);

  j_data = json_object();
  json_object_set_new(j_data, "name", json_string("Ned Stark Winter is coming"));
  json_object_set_new(j_data, "age", json_integer(45));
  json_object_set_new(j_data, "temperature", json_real(30.1));
  json_object_set_new(j_data, "birthdate", json_string("1408-06-01 03:05:11"));
  
  j_query = json_object();
  json_object_set_new(j_query, "table", json_string(table));
  json_object_set_new(j_query, "values", j_data);
  
  printf("insert result: %d\n", h_insert(conn, j_query, NULL));
  json_decref(j_query);
  
  j_array = json_array();
  json_array_append_new(j_array, json_string("name"));
  json_array_append_new(j_array, json_string("age"));
  json_array_append_new(j_array, json_string("birthdate"));
  j_where = json_object();
  json_object_set_new(j_where, "name", json_string("Ned Stark Winter is coming"));
  
  j_query = json_object();
  json_object_set_new(j_query, "table", json_string(table));
  json_object_set_new(j_query, "columns", j_array);
  json_object_set_new(j_query, "where", j_where);
  
  if (h_select(conn, j_query, &j_result, NULL) == H_OK) {
    dump = json_dumps(j_result, JSON_INDENT(2));
    printf("json select result is\n%s\n", dump);
    json_decref(j_result);
    free(dump);
  } else {
    printf("Error executing select query\n");
  }
  json_decref(j_query);
  
  j_where = json_object();
  json_object_set_new(j_where, "age", json_integer(45));
  j_set = json_object();
  json_object_set_new(j_set, "age", json_integer(47));
  
  j_query = json_object();
  json_object_set_new(j_query, "table", json_string(table));
  json_object_set_new(j_query, "set", j_set);
  json_object_set_new(j_query, "where", j_where);
  
  if (h_update(conn, j_query, NULL) == H_OK) {
    printf("Update query OK\n");
    json_decref(j_query);
    j_query = json_object();
    json_object_set_new(j_query, "table", json_string(table));
    if (h_select(conn, j_query, &j_result, NULL) == H_OK) {
      dump = json_dumps(j_result, JSON_INDENT(2));
      printf("json select result is\n%s\n", dump);
      json_decref(j_result);
      free(dump);
    } else {
      printf("Error executing select query\n");
    }
    json_decref(j_query);
  } else {
    json_decref(j_query);
    printf("Error executing update query\n");
  }
  
  j_where = json_object();
  json_object_set_new(j_where, "age", json_pack("{sssi}", "operator", ">", "value", 46));
  
  j_query = json_object();
  json_object_set_new(j_query, "table", json_string(table));
  json_object_set_new(j_query, "where", j_where);
  
  if (h_delete(conn, j_query, NULL) == H_OK) {
    printf("Delete query OK\n");
    json_decref(j_query);
    j_query = json_object();
    json_object_set_new(j_query, "table", json_string(table));
    if (h_select(conn, j_query, &j_result, NULL) == H_OK) {
      dump = json_dumps(j_result, JSON_INDENT(2));
      printf("json select result is\n%s\n", dump);
      json_decref(j_result);
      free(dump);
    } else {
      printf("Error executing select query\n");
    }
  } else {
    json_decref(j_query);
    printf("Error executing delete query\n");
  }
}

int main(int argc, char ** argv) {
  struct _h_connection * conn;
  
  y_init_logs("test_hoel_mariadb_json", Y_LOG_MODE_CONSOLE, Y_LOG_LEVEL_DEBUG, NULL, "Starting test_hoel_mariadb_json");
  
  conn = h_connect_mariadb("lohot", "test_hoel", "test_hoel", "test_hoel", 0, NULL);
  
  if (conn != NULL) {
    unit_tests(conn);
  } else {
    printf("Error connecting to database\n");
  }
  h_close_db(conn);
  
  y_close_logs();
  
  return h_clean_connection(conn);
}
