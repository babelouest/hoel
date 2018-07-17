/**
 * 
 * Example program using json-based queries to execute SQL statements
 * 
 * Copyright 2016-2018 Nicolas Mora <mail@babelouest.org>
 *
 * License: MIT
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <jansson.h>

#define _HOEL_MARIADB
#include <hoel.h>

void hoel_json_tests(struct _h_connection * conn) {
  json_t * j_result, * j_query;
  char * table = "other_test", * dump;
  int res;
  
  /*
   *
   * First select query
   *
   * The generated SQL query will be:
   * SELECT name,age,temperature FROM other_test WHERE age>46 AND name LIKE 'Hodor%';
   *
   */
  j_query = json_pack("{sss[sss]s{s{sssi}s{ssss}}}",
                      "table",
                      table,
                      "columns",
                        "name",
                        "age",
                        "temperature",
                      "where",
                        "age",
                          "operator",
                          ">",
                          "value",
                          46,
                        "name",
                          "operator",
                          "LIKE",
                          "value",
                          "Hodor%");
  
  // Execute the query
  res = h_select(conn, j_query, &j_result, NULL);
  // Deallocate j_query since it won't be needed anymore
  json_decref(j_query);
  // Test query execution result
  if (res == H_OK) {
    // Print result
    dump = json_dumps(j_result, JSON_INDENT(2));
    y_log_message(Y_LOG_LEVEL_DEBUG, "json select result is\n%s", dump);
    // Deallocate data result
    json_decref(j_result);
    free(dump);
  } else {
    y_log_message(Y_LOG_LEVEL_DEBUG, "Error executing select query: %d", res);
  }

  /*
   *
   * Insert query
   *
   * The generated SQL query will be:
   * INSERT INTO other_test (name,age,temperature,birthdate) VALUES ('Ned Stark Winter is coming',45,30.1,'1408-06-01 03:05:11');
   *
   */
  j_query = json_pack("{sss{sssisfss}}",
                      "table",
                      table,
                      "values",
                        "name",
                        "Ned Stark Winter is coming",
                        "age",
                        45,
                        "temperature",
                        30.1,
                        "birthdate",
                        "1408-06-01 03:05:11");
  y_log_message(Y_LOG_LEVEL_DEBUG, "insert result: %d", h_insert(conn, j_query, NULL)); // Expected result: H_OK (0)
  json_decref(j_query);
  
  /*
   *
   * Second select query
   *
   * The generated SQL query will be:
   * SELECT name,age,temperature FROM other_test WHERE name='Ned Stark Winter is coming';
   *
   * This select query is supposed to return at least one result
   *
   */
  j_query = json_pack("{sss[sss]s{ss}}",
                      "table",
                      table,
                      "columns",
                        "name",
                        "age",
                        "birthdate",
                      "where",
                        "name",
                        "Ned Stark Winter is coming");
  if (h_select(conn, j_query, &j_result, NULL) == H_OK) {
    dump = json_dumps(j_result, JSON_INDENT(2));
    y_log_message(Y_LOG_LEVEL_DEBUG, "json select result is\n%s", dump);
    json_decref(j_result);
    free(dump);
  } else {
    y_log_message(Y_LOG_LEVEL_DEBUG, "Error executing select query");
  }
  json_decref(j_query);
  
  /*
   *
   * Update query
   *
   * The generated SQL query will be:
   * UPDATE other_test SET age=47 WHERE age=45;
   *
   */
  j_query = json_pack("{sss{si}s{si}}",
                      "table",
                      table,
                      "set",
                        "age",
                        47,
                      "where",
                        "age",
                        45);
  
  if (h_update(conn, j_query, NULL) == H_OK) {
    y_log_message(Y_LOG_LEVEL_DEBUG, "Update query OK");
    json_decref(j_query);
    j_query = json_pack("{ss}", "table", table);
    if (h_select(conn, j_query, &j_result, NULL) == H_OK) {
      dump = json_dumps(j_result, JSON_INDENT(2));
      y_log_message(Y_LOG_LEVEL_DEBUG, "json select result is\n%s", dump);
      json_decref(j_result);
      free(dump);
    } else {
      y_log_message(Y_LOG_LEVEL_DEBUG, "Error executing select query");
    }
    json_decref(j_query);
  } else {
    json_decref(j_query);
    y_log_message(Y_LOG_LEVEL_DEBUG, "Error executing update query");
  }
  
  /*
   *
   * Delete query
   *
   * The generated SQL query will be:
   * DELETE FROM other_test WHERE age>46;
   *
   */
  j_query = json_pack("{sss{s{sssi}}}",
                      "table",
                      table,
                        "where",
                          "age",
                            "operator",
                            ">",
                            "value",
                            46);
  
  if (h_delete(conn, j_query, NULL) == H_OK) {
    y_log_message(Y_LOG_LEVEL_DEBUG, "Delete query OK");
    json_decref(j_query);
    j_query = json_pack("{ss}", "table", table);
    if (h_select(conn, j_query, &j_result, NULL) == H_OK) {
      dump = json_dumps(j_result, JSON_INDENT(2));
      y_log_message(Y_LOG_LEVEL_DEBUG, "json select result is\n%s", dump);
      json_decref(j_result);
      free(dump);
    } else {
      y_log_message(Y_LOG_LEVEL_DEBUG, "Error executing select query");
    }
  } else {
    json_decref(j_query);
    y_log_message(Y_LOG_LEVEL_DEBUG, "Error executing delete query");
  }
}

int main() {
  struct _h_connection * conn;
  
  // Initialize logs manager to console output
  y_init_logs("test_hoel_mariadb_json", Y_LOG_MODE_CONSOLE, Y_LOG_LEVEL_DEBUG, NULL, "Starting test_hoel_mariadb_json");
  
  // Create hoel connection
  conn = h_connect_mariadb("lamorak", "test_hoel", "test_hoel", "test_hoel", 0, NULL);
  
  if (conn != NULL) {
    // Execute hoel_json_tests function that will run the tests themselves
    hoel_json_tests(conn);
  } else {
    y_log_message(Y_LOG_LEVEL_DEBUG, "Error connecting to database");
  }
  // Close database connection
  h_close_db(conn);
  
  // Close logs manager
  y_close_logs();
  
  // Clear hoel connection
  return h_clean_connection(conn);
}
