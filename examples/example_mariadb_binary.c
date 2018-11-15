#include <hoel.h>
#include <yder.h>

// The table structure is: 'create table test( integer_col int(10), string_col varchar(128), blob_col blob, binary_col binary(16));'

int main() {
  struct _h_connection * conn = h_connect_mariadb("localhost", "test", "test", "test_binary", 0, NULL);

  y_init_logs("test binary", Y_LOG_MODE_CONSOLE, Y_LOG_LEVEL_DEBUG, NULL, "Starting test binary");
  if (conn != NULL) {
    struct _h_result result;
    char * query = "insert into test (integer_col, string_col, blob_col, binary_col) values (42, 'nope', 'contribution', 'a')";
    y_log_message(Y_LOG_LEVEL_DEBUG, "Insert result: %d", h_query_insert(conn, query));
    query = "select * from test";
    if (h_query_select(conn, query, &result) == H_OK) {
      int row, col;
      for (row=0; row<result.nb_rows; row++) {
        for (col=0; col<result.nb_columns; col++) {
          if (result.data[row][col].type == HOEL_COL_TYPE_INT) {
            y_log_message(Y_LOG_LEVEL_DEBUG, "int value: %d", ((struct _h_type_int *)result.data[row][col].t_data)->value);
          } else if (result.data[row][col].type == HOEL_COL_TYPE_TEXT) {
            y_log_message(Y_LOG_LEVEL_DEBUG, "text value: %.*s", ((struct _h_type_text *)result.data[row][col].t_data)->length, ((struct _h_type_text *)result.data[row][col].t_data)->value);
          } else if (result.data[row][col].type == HOEL_COL_TYPE_BLOB) {
            y_log_message(Y_LOG_LEVEL_DEBUG, "blob value: %.*s", ((struct _h_type_blob *)result.data[row][col].t_data)->length, ((struct _h_type_blob *)result.data[row][col].t_data)->value);
          }
        }
      }
    } else {
      y_log_message(Y_LOG_LEVEL_DEBUG, "Error executing select");
    }
    query = "delete from test";
    y_log_message(Y_LOG_LEVEL_DEBUG, "Delete result: %d", h_query_delete(conn, query));
  } else {
    y_log_message(Y_LOG_LEVEL_DEBUG, "Error connecting to database");
  }
  y_close_logs();
  h_close_db(conn);
}
