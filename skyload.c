/* 
 * Copyright (C) 2009 Toru Maesaka <dev@torum.net>
 * All Rights Reserved.
 *
 * Use and distribution of this program is licensed under the
 * BSD license. See the COPYING file for full text.
 */

#include "skyload.h"
#include "generator.h"

static bool create_skyload_database(SKY_SHARE *share) {
  assert(share);

  drizzle_st drizzle;
  drizzle_con_st connection;
  drizzle_return_t ret;
  drizzle_result_st result;

  drizzle_create(&drizzle);
  
  if (!sky_create_connection(share, &drizzle, &connection)) {
    report_error("failed to initialize connection");
    drizzle_free(&drizzle);
    return false;
  }

  /* Attempt to drop the database just in case the database
     still exists from the previous run. */
  if (!drop_database(share)) {
    report_error("failed to drop database");
    drizzle_free(&drizzle);
    return false;
  }

  /* Now we actually create the database */
  drizzle_query_str(&connection, &result, SKY_DB_CREATE, &ret);

  if (ret != DRIZZLE_RETURN_OK) {
    report_error(drizzle_con_error(&connection));
    drizzle_free(&drizzle);
    return false;
  }

  drizzle_result_free(&result);

  /* Switch to the database that we want to work in */
  if (!switch_database(share, &connection)) {
    report_error(drizzle_con_error(&connection));
    drizzle_free(&drizzle);
    return false;
  }

  /* Create the test table */
  if (share->create_query)
    drizzle_query_str(&connection, &result, share->create_query, &ret);

  if (ret != DRIZZLE_RETURN_OK) {
    report_error(drizzle_con_error(&connection));
    drizzle_free(&drizzle);
    return false;
  }
  drizzle_result_free(&result);

  sky_close_connection(&connection);
  drizzle_free(&drizzle);
  return true;
}

static bool insert_benchmark(SKY_WORKER *context) {
  assert(context);

  struct timeval start_time;
  struct timeval end_time;
  char query_buf[SKY_STRSIZ];
  drizzle_result_st result;
  drizzle_return_t ret;

  uint32_t nwrite = rows_to_write(context);
  if (context->unique_id == 1)
    fprintf(stdout, "Skyload Worker[0] INSERT Progress:\n");

  for (int i = 0; i < nwrite; i++) {
    size_t qlen = next_insert_query(context, query_buf, SKY_STRSIZ);

    if (qlen <= 0) {
      fprintf(stderr, "thread[%d] invalid INSERT template\n",
              context->unique_id);
      sky_close_connection(&context->connection);      
      context->aborted = true;
      return NULL;
    }

    /* Attempt to insert the generated INSERT query */
    gettimeofday(&start_time, NULL);
    drizzle_query_str(&context->connection, &result, query_buf, &ret);

    if (ret != DRIZZLE_RETURN_OK) {
      fprintf(stderr, "thread[%d] error: %s\n",
              context->unique_id, drizzle_con_error(&context->connection));
      context->aborted = true;
      sky_close_connection(&context->connection);
      return NULL;
    }
    drizzle_result_free(&result);

    /* accumulate the time it took to execute this query for later
       aggregation by the main thread */
    gettimeofday(&end_time, NULL);
    context->total_insert_time += timediff(end_time, start_time);

    /* Print the progress of the first worker thread so we can give
       some feedback to the user. Progress feedback for all worker
       threads in a single feed would be nice but this requires
       atomic increment or use of mutex which can potentially reduce
       the effectiveness of the load test. */
    if (context->unique_id == 1) {
      if (nwrite > 25) {
        if(((i + 1) % 25) == 0) {
          putchar('.');
          fflush(stdout);
        }
      }
      if (((i + 1) % 1000) == 0 || i == nwrite-1)
        fprintf(stdout, " (%d)\n", i + 1);
    }
  }
  return true;
}

static bool sql_file_benchmark(SKY_WORKER *context) {
  assert(context && context->share->read_queries);

  SKY_LIST_NODE *current = context->share->read_queries->head;
  struct timeval start_time;
  struct timeval end_time;
  drizzle_result_st result;
  drizzle_return_t ret;

  for (int i = 0; i < context->share->read_queries->size; i++) {
    gettimeofday(&start_time, NULL);
    drizzle_query_str(&context->connection, &result, current->data, &ret);

    if (ret != DRIZZLE_RETURN_OK) {
      fprintf(stderr, "thread[%d] error: %s\n",
              context->unique_id, drizzle_con_error(&context->connection));
      context->aborted = true;
      sky_close_connection(&context->connection);
      return false;
    }

    ret = drizzle_result_buffer(&result);

    if (ret != DRIZZLE_RETURN_OK) {
      fprintf(stderr, "thread[%d] error: %s\n",
              context->unique_id, drizzle_con_error(&context->connection));
      context->aborted = true;
      sky_close_connection(&context->connection);
      return false;
    }

    drizzle_result_free(&result);
    gettimeofday(&end_time, NULL);
    context->file_benchmark_time += timediff(end_time, start_time);

    current = current->next;
  }
  return true;
}

void *workload(void *arg) {
  assert(arg);

  SKY_WORKER *context = (SKY_WORKER *)arg;

  /* Initialize worker specific connection */
  if (!sky_create_connection(context->share, &context->database_handle,
                             &context->connection)) {
    report_error("failed to initialize connection");
    context->aborted = true;
    drizzle_free(&context->database_handle);
    pthread_exit(NULL);
  }

  /* Switch to the test database */
  if (!switch_database(context->share, &context->connection)) {
    report_error(drizzle_con_error(&context->connection));
    context->aborted = true;
    drizzle_free(&context->database_handle);
    pthread_exit(NULL);
  }

  /* Perform insertion benchmark if speficified */
  if (context->share->insert_tmpl && context->share->nwrite > 0) {
    if (!insert_benchmark(context))
      pthread_exit(NULL);
    if (context->unique_id == 1) {
      fprintf(stdout, "\n");
      fprintf(stdout, "Populating DB with auto generated data: Done\n");
    }
  }

  /* Run benchmark based on the supplied SQL file */
  if (context->share->read_queries && context->share->read_queries->size > 0) {
    if (context->unique_id == 1) {
      fprintf(stdout, "Emulating Read Load: ");
    }

    for (int i = 0; i < context->share->runs; i++) {
      if (!sql_file_benchmark(context))
        pthread_exit(NULL);
    }

    if (context->unique_id == 1)
      fprintf(stdout, "Done\n");
  }

  sky_close_connection(&context->connection);
  return NULL;
}

int main(int argc, char **argv) {
  SKY_SHARE *share;
  SKY_WORKER **workers;
  pthread_attr_t joinable;

  if (argc == 1)
    usage();

  /* This object is shared among all worker threads */
  if ((share = sky_share_new()) == NULL) {
    report_error("out of memory");
    return EXIT_FAILURE;
  }

  /* Get user options and set it to share */
  if (!handle_options(share, argc, argv)) {
    sky_share_free(share);
    return EXIT_FAILURE;
  }

  /* Check if the provided options make sense */
  if (!check_options(share)) {
    sky_share_free(share);
    return EXIT_FAILURE;
  }

  /* Use the appropriate port if unspecified */
  if (share->port == 0) {
    if (share->protocol == DRIZZLE_CON_MYSQL)
      share->port = MYSQL_DEFAULT_PORT;
    else
      share->port = DRIZZLE_DEFAULT_PORT;
  }

  /* If provided, load the external SQL file to memory */
  if (!preload_sql_file(share)) {
    sky_share_free(share);
    return EXIT_FAILURE;
  }

  /* Create worker object(s) */
  if ((workers = create_workers(share)) == NULL) {
    report_error("out of memory");
    return EXIT_FAILURE;
  }

  /* Create a new database if one isn't specified */
  if (share->database_name == NULL) {
    if (create_skyload_database(share) == false) {
      sky_share_free(share);
      return EXIT_FAILURE;
    }
  } 

  /* If provided, load the file content to the database */
  if (share->load_file_path && share->load_queries) {
    if (!preload_database(share)) {
      sky_share_free(share);
      return EXIT_FAILURE;
    }
  }

  pthread_attr_init(&joinable);
  pthread_attr_setdetachstate(&joinable, PTHREAD_CREATE_JOINABLE);

  /* Start benchmarking */
  for (int i = 0; i < share->concurrency; i++) {
    if (pthread_create(&workers[i]->thread_id, &joinable, workload,
                       (void *)workers[i])) {
      report_error("failed to create worker thread");
      return EXIT_FAILURE;
    }
  }

  pthread_attr_destroy(&joinable);

  /* Wait for threads to finish their workout */
  for (int i = 0; i < share->concurrency; i++) {
    if (pthread_join(workers[i]->thread_id, NULL) != 0) {
      report_error("error while waiting for threads to terminate.");
      return EXIT_FAILURE;
    }
  }

  /* Aggregate and print the benchmark result held by all workers */ 
  aggregate_worker_result(workers);

  /* skyload is done, drop the database unless we're specified not
     to or if we're using an existing database */
  if (!share->keep_db && !share->database_name) {
    if (drop_database(share) == false)
      return EXIT_FAILURE;
  }

  if (share->read_queries != NULL)
    sky_list_free(share->read_queries);
  if (share->load_queries != NULL)
    sky_list_free(share->load_queries);

  destroy_workers(workers);
  sky_share_free(share);
  return EXIT_SUCCESS;
}
