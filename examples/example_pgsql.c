/**
 * 
 * Example program using PostgreSQL to execute SQL statements
 * 
 * Copyright 2016-2018 Nicolas Mora <mail@babelouest.org>
 *
 * License: MIT
 *
 */
#include <stdio.h>
#include <jansson.h>
#include <yder.h>

#define _HOEL_PGSQL
#include <hoel.h>

void print_result(struct _h_result result) {
  int col, row, i;
  char buf[64];
  y_log_message(Y_LOG_LEVEL_DEBUG, "rows: %d, col: %d", result.nb_rows, result.nb_columns);
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
        case HOEL_COL_TYPE_DATE:
          strftime(buf, 64, "%Y-%m-%d %H:%M:%S", &((struct _h_type_datetime *)result.data[row][col].t_data)->value);
          printf("| %s ", buf);
        case HOEL_COL_TYPE_NULL:
          printf("| [null] ");
          break;
      }
    }
    printf("|\n");
  }
}

int main(int argc, char ** argv) {
  struct _h_result result;
  struct _h_connection * conn;
  char * query = "select * from test",
			 * insert_query = "insert into test (name, age, birthdate) values ('bob', 21, '1997-05-09')",
			 * connectionstring = "host=localhost dbname=test user=test password=test",
			 * dump = NULL;
  int res;
  json_t * j_result;
  
	y_init_logs("example_pgsql", Y_LOG_MODE_CONSOLE, Y_LOG_LEVEL_DEBUG, NULL, "Starting example_pgsql");
	
  conn = h_connect_pgsql(connectionstring);
	
	res = h_query_insert(conn, insert_query);
	
  if (res == H_OK) {
    y_log_message(Y_LOG_LEVEL_DEBUG, "insert query executed");
		j_result = h_last_insert_id(conn);
		dump = json_dumps(j_result, JSON_ENCODE_ANY);
		y_log_message(Y_LOG_LEVEL_DEBUG, "last id is %s", dump);
		free(dump);
		json_decref(j_result);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error executing query: %d", res);
  }
	
  res = h_query_select(conn, query, &result);
  
  if (res == H_OK) {
    print_result(result);
    h_clean_result(&result);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error executing query: %d", res);
  }
  
  res = h_execute_query_json(conn, query, &j_result);
  
  if (res == H_OK) {
    dump = json_dumps(j_result, JSON_INDENT(2));
    y_log_message(Y_LOG_LEVEL_DEBUG, "json result is\n%s", dump);
    json_decref(j_result);
    free(dump);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error executing json query: %d", res);
  }
  h_close_db(conn);
	y_close_logs();
	
  return h_clean_connection(conn);
}
