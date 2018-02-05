#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <jansson.h>
#define _HOEL_MARIADB
#include "../include/hoel.h"

#define NB_THREADS 5

struct thread_arg {
	struct _h_connection * conn;
	unsigned int id;
};

static void clean_database(struct _h_connection * conn) {
	json_t * j_query = json_pack("{ss}", "table", "other_test");
	
	if (h_delete(conn, j_query, NULL) != H_OK) {
		y_log_message(Y_LOG_LEVEL_ERROR, "Error deleting table");
	}
	
	json_decref(j_query);
}

void * thread_run_query(void * args) {
	struct _h_connection * conn = ((struct thread_arg *)args)->conn;
	json_t * j_query, * j_result = NULL;
	int i, j;
	
	j_query = json_pack("{sss[]}", "table", "other_test", "values");
	
	for (i=0; i < 5; i++) {
		y_log_message(Y_LOG_LEVEL_INFO, "Start iteration %d on id %d, add %d elements", i, ((struct thread_arg *)args)->id, 100*(i+1));
		
		for (j=0; j<100*(i+1); j++) {
			json_array_append_new(json_object_get(j_query, "values"), json_pack("{si}", "age", j));
		}
		
		if (h_insert(conn, j_query, NULL) != H_OK) {
			y_log_message(Y_LOG_LEVEL_ERROR, "Error insert table, id %d, iteration %d", ((struct thread_arg *)args)->id, i);
		} else {
			y_log_message(Y_LOG_LEVEL_INFO, "id %d, iteration %d, insert OK", ((struct thread_arg *)args)->id, i);
		}
		
		if (h_select(conn, j_query, &j_result, NULL) != H_OK) {
			y_log_message(Y_LOG_LEVEL_ERROR, "Error select table, id %d, iteration %d", ((struct thread_arg *)args)->id, i);
		} else {
			y_log_message(Y_LOG_LEVEL_INFO, "id %d, iteration %d, count %zu", ((struct thread_arg *)args)->id, i, json_array_size(j_result));
			json_decref(j_result);
			j_result = NULL;
		}
		
		y_log_message(Y_LOG_LEVEL_INFO, "End iteration %d on id %d", i, ((struct thread_arg *)args)->id);
	}
	json_decref(j_query);
	return NULL;
}

int main(int argc, char ** argv) {
  struct _h_connection * conn;
	int i;
  pthread_t query_thread[NB_THREADS];
	struct thread_arg args[NB_THREADS];
	
  y_init_logs("test_hoel_mariadb_json", Y_LOG_MODE_CONSOLE, Y_LOG_LEVEL_DEBUG, NULL, "Starting test_multi_mariadb");
  
  conn = h_connect_mariadb("girflet", "test_hoel", "test_hoel", "test_hoel", 0, NULL);
  
  if (conn != NULL) {
		// Create and run threads
		for (i=0; i<NB_THREADS; i++) {
			args[i].id = i;
			args[i].conn = conn;
			pthread_create(&query_thread[i], NULL, thread_run_query, &args[i]);
		}
		
		// Wait for all threads to end
		for (i=0; i<NB_THREADS; i++) {
			pthread_join(query_thread[i], NULL);
		}
		
		clean_database(conn);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error connecting to database");
  }
  h_close_db(conn);
  
  y_close_logs();
  
  return h_clean_connection(conn);
}
