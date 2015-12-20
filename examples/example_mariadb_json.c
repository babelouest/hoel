#include <stdio.h>
#include <stdlib.h>
#include <jansson.h>
#define _HOEL_MARIADB
#include "../src/hoel.h"

void unit_tests(struct _h_connection * conn) {
  json_t * j_result, * j_where, * j_array, * j_set, * j_data;
  char * table = "other_test", * dump;
  
  j_data = json_object();
  json_object_set_new(j_data, "name", json_string("Ned Stark Winter is coming"));
  json_object_set_new(j_data, "age", json_integer(45));
  json_object_set_new(j_data, "temperature", json_real(30.1));
  json_object_set_new(j_data, "birthdate", json_string("1408-06-01 03:05:11"));
  
  printf("insert result: %d\n", h_insert(conn, table, j_data));
  json_decref(j_data);
  
  j_array = json_array();
  json_array_append_new(j_array, json_string("name"));
  json_array_append_new(j_array, json_string("age"));
  json_array_append_new(j_array, json_string("birthdate"));
  j_where = json_object();
  json_object_set_new(j_where, "name", json_string("Ned Stark Winter is coming"));
  if (h_select(conn, table, j_array, j_where, &j_result) == H_OK) {
    dump = json_dumps(j_result, JSON_INDENT(2));
    printf("json select result is\n%s\n", dump);
    json_decref(j_result);
    free(dump);
  } else {
    printf("Error executing select query\n");
  }
  json_decref(j_array);
  json_decref(j_where);
  
  j_where = json_object();
  json_object_set_new(j_where, "age", json_integer(45));
  j_set = json_object();
  json_object_set_new(j_set, "age", json_integer(47));
  if (h_update(conn, table, j_set, j_where) == H_OK) {
    printf("Update query OK\n");
    if (h_select(conn, table, NULL, NULL, &j_result) == H_OK) {
      dump = json_dumps(j_result, JSON_INDENT(2));
      printf("json select result is\n%s\n", dump);
      json_decref(j_result);
      free(dump);
    } else {
      printf("Error executing select query\n");
    }
  } else {
    printf("Error executing update query\n");
  }
  json_decref(j_set);
  json_decref(j_where);
  
  j_where = json_object();
  json_object_set_new(j_where, "age", json_integer(47));
  if (h_delete(conn, table, j_where) == H_OK) {
    printf("Delete query OK\n");
    if (h_select(conn, table, NULL, NULL, &j_result) == H_OK) {
      dump = json_dumps(j_result, JSON_INDENT(2));
      printf("json select result is\n%s\n", dump);
      json_decref(j_result);
      free(dump);
    } else {
      printf("Error executing select query\n");
    }
  } else {
    printf("Error executing delete query\n");
  }
  json_decref(j_where);
}

int main(int argc, char ** argv) {
  struct _h_connection * conn;
  
  conn = h_connect_mariadb("localhost", "test_hoel", "test_hoel", "test_hoel", 0, NULL);
  
  if (conn != NULL) {
    unit_tests(conn);
  } else {
    printf("Error connecting to database\n");
  }
  h_close_db(conn);
  return h_clean_connection(conn);
}
