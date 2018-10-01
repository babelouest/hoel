#include <stdio.h>
#include <stdlib.h>
#include <jansson.h>
#include <yder.h>
#define _HOEL_SQLITE
#include <hoel.h>

void print_result(struct _h_result result) {
  int col, row, i;
  printf("rows: %u, col: %u\n", result.nb_rows, result.nb_columns);
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
  char * query = NULL, * sanitized = NULL, * table = "other_test";
  int last_id = -1;
  
  query = msprintf("select * from %s", table);
  if (h_query_select(conn, query, &result) == H_OK) {
    printf("\n\nIteration 1, initial status\n");
    print_result(result);
    h_clean_result(&result);
  } else {
    printf("Error executing query\n");
  }
  free(query);
  
  sanitized = h_escape_string(conn, "Hodor");
  query = msprintf("insert into %s (name, age, temperature) values ('%s', %d, %f)", table, sanitized, 33, 37.2);
  printf("insert result: %d\n", h_query_insert(conn, query));
  free(sanitized);
  free(query);
  
  query = msprintf("select * from %s", table);
  if (h_query_select(conn, query, &result) == H_OK) {
    printf("\n\nIteration 2, after insert\n");
    print_result(result);
    h_clean_result(&result);
  } else {
    printf("Error executing query\n");
  }
  free(query);
  
  sanitized = h_escape_string(conn, "Ygritte");
  query = msprintf(NULL, 0, "insert into %s (name, age, temperature) values ('%s', %d, %f)", table, sanitized, 25, 30.1);
  printf("insert result: %d\n", h_query_insert(conn, query));
  free(sanitized);
  free(query);
  
  query = msprintf("select * from %s", table);
  if (h_query_select(conn, query, &result) == H_OK) {
    printf("\n\nIteration 3, after insert\n");
    print_result(result);
    h_clean_result(&result);
  } else {
    printf("Error executing query\n");
  }
  free(query);
  
  sanitized = h_escape_string(conn, "Littlefinger");
  query = msprintf("insert into %s (name, age, temperature) values ('%s', %d, %f)", table, sanitized, 44, 40.5);
  printf("insert result: %d\n", h_query_insert(conn, query));
  free(sanitized);
  free(query);
  data = h_query_last_insert_id(conn);
  if (data->type == HOEL_COL_TYPE_INT) {
    last_id = ((struct _h_type_int *)data->t_data)->value;
  }
  h_clean_data_full(data);
  printf("last id is %d\n", last_id);
  
  query = msprintf("select * from %s", table);
  if (h_query_select(conn, query, &result) == H_OK) {
    printf("\n\nIteration 4, after inserts and last id\n");
    print_result(result);
    h_clean_result(&result);
  } else {
    printf("Error executing query\n");
  }
  free(query);
  
  sanitized = h_escape_string(conn, "Littlefingers");
  query = msprintf("update %s set name='%s' where id=%d", table, sanitized, last_id);
  printf("update result: %d\n", h_query_update(conn, query));
  free(sanitized);
  free(query);

  query = msprintf("select * from %s", table);
  if (h_query_select(conn, query, &result) == H_OK) {
    printf("\n\nIteration 5, after update\n");
    print_result(result);
    h_clean_result(&result);
  } else {
    printf("Error executing query\n");
  }
  free(query);
  
  query = msprintf("delete from %s where id=%d", table, last_id);
  printf("delete result: %d\n", h_query_delete(conn, query));
  free(query);
  
  query = msprintf("select * from %s", table);
  if (h_query_select(conn, query, &result) == H_OK) {
    printf("\n\nIteration 6, after delete\n");
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
  
  y_init_logs("test_hoel_sqlite3", Y_LOG_MODE_CONSOLE, Y_LOG_LEVEL_DEBUG, NULL, "Starting test_hoel_sqlite3");
  conn = h_connect_sqlite(db_file);
  
  if (conn != NULL) {
    unit_tests(conn);
  }
  h_close_db(conn);
  
  y_close_logs();
  
  return h_clean_connection(conn);
}
