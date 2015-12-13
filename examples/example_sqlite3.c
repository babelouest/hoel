#include <stdio.h>
#include <stdlib.h>
#define _HOEL_SQLITE
#include "../src/hoel.h"

void print_result(struct _h_result result) {
  int col, row, i;
  printf("rows: %d, col: %d\n", result.nb_rows, result.nb_columns);
  for (row = 0; row<result.nb_rows; row++) {
    for (col=0; col<result.nb_columns; col++) {
      switch(result.data[row][col].type) {
        case HOEL_COL_TYPE_INT:
          printf("| %d ", ((struct _h_type_int *)result.data[row][col].t_data)->value);
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

void unit_tests(struct _h_connection * conn) {
  struct _h_result result;
  struct _h_data * data;
  char * query = NULL, * sanitized = NULL;
  int len, last_id;
  
  len = snprintf(NULL, 0, "select * from other_test");
  query = malloc((len+1)*sizeof(char));
  snprintf(query, (len+1), "select * from other_test");
  if (h_query_select(conn, query, &result) == H_OK) {
    print_result(result);
    h_clean_result(&result);
  } else {
    printf("Error executing query\n");
  }
  free(query);
  
  sanitized = h_escape_string(conn, "Hodor son of H'rtp'ss");
  len = snprintf(NULL, 0, "insert into other_test (name, age, temperature) values ('%s', %d, %f)", sanitized, 33, 37.2);
  query = malloc((len+1)*sizeof(char));
  snprintf(query, (len+1), "insert into other_test (name, age, temperature) values ('%s', %d, %f)", sanitized, 33, 37.2);
  printf("insert result: %d\n", h_query_insert(conn, query));
  free(sanitized);
  free(query);
  
  len = snprintf(NULL, 0, "select * from other_test");
  query = malloc((len+1)*sizeof(char));
  snprintf(query, (len+1), "select * from other_test");
  if (h_query_select(conn, query, &result) == H_OK) {
    print_result(result);
    h_clean_result(&result);
  } else {
    printf("Error executing query\n");
  }
  free(query);
  
  sanitized = h_escape_string(conn, "Ygritte you know nothing");
  len = snprintf(NULL, 0, "insert into other_test (name, age, temperature) values ('%s', %d, %f)", sanitized, 25, 30.1);
  query = malloc((len+1)*sizeof(char));
  snprintf(query, (len+1), "insert into other_test (name, age, temperature) values ('%s', %d, %f)", sanitized, 25, 30.1);
  printf("insert result: %d\n", h_query_insert(conn, query));
  free(sanitized);
  free(query);
  
  len = snprintf(NULL, 0, "select * from other_test");
  query = malloc((len+1)*sizeof(char));
  snprintf(query, (len+1), "select * from other_test");
  if (h_query_select(conn, query, &result) == H_OK) {
    print_result(result);
    h_clean_result(&result);
  } else {
    printf("Error executing query\n");
  }
  free(query);
  
  sanitized = h_escape_string(conn, "Littlefinger I will betray you");
  len = snprintf(NULL, 0, "insert into other_test (name, age, temperature) values ('%s', %d, %f)", sanitized, 44, 40.5);
  query = malloc((len+1)*sizeof(char));
  snprintf(query, (len+1), "insert into other_test (name, age, temperature) values ('%s', %d, %f)", sanitized, 44, 40.5);
  printf("insert result: %d\n", h_query_insert(conn, query));
  free(sanitized);
  free(query);
  data = h_last_insert_id(conn);
  last_id = ((struct _h_type_int *)data->t_data)->value;
  h_clean_data_full(data);
  printf("last id is %d\n", last_id);
  
  len = snprintf(NULL, 0, "select * from other_test");
  query = malloc((len+1)*sizeof(char));
  snprintf(query, (len+1), "select * from other_test");
  if (h_query_select(conn, query, &result) == H_OK) {
    print_result(result);
    h_clean_result(&result);
  } else {
    printf("Error executing query\n");
  }
  free(query);
  
  sanitized = h_escape_string(conn, "Littlefinger I am nothing");
  len = snprintf(NULL, 0, "update other_test set name='%s' where id=%d", sanitized, last_id);
  query = malloc((len+1)*sizeof(char));
  snprintf(query, (len+1), "update other_test set name='%s' where id=%d", sanitized, last_id);
  printf("update result: %d\n", h_query_update(conn, query));
  free(sanitized);
  free(query);

  len = snprintf(NULL, 0, "select * from other_test");
  query = malloc((len+1)*sizeof(char));
  snprintf(query, (len+1), "select * from other_test");
  if (h_query_select(conn, query, &result) == H_OK) {
    print_result(result);
    h_clean_result(&result);
  } else {
    printf("Error executing query\n");
  }
  free(query);
  
  len = snprintf(NULL, 0, "delete from other_test where id=%d", last_id);
  query = malloc((len+1)*sizeof(char));
  snprintf(query, (len+1), "delete from other_test where id=%d", last_id);
  printf("delete result: %d\n", h_query_delete(conn, query));
  free(query);
  
  len = snprintf(NULL, 0, "select * from other_test");
  query = malloc((len+1)*sizeof(char));
  snprintf(query, (len+1), "select * from other_test");
  if (h_query_select(conn, query, &result) == H_OK) {
    print_result(result);
    h_clean_result(&result);
  } else {
    printf("Error executing query\n");
  }
  free(query);
}

int main(int argc, char ** argv) {
  struct _h_connection * conn;
  char * db_file = "/tmp/test.db";
  
  conn = h_connect_sqlite(db_file);
  
  if (conn != NULL) {
    unit_tests(conn);
  }
  h_close_db(conn);
  return h_clean_connection(conn);
}
