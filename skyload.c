/* 
 * Copyright (C) 2009 Toru Maesaka <dev@torum.net>
 * All Rights Reserved.
 *
 * Use and distribution of this program is licensed under the
 * BSD license. See the COPYING file for full text.
 */

#include "skyload.h"
#include "generator.h"

static bool switch_to_skyload_database(drizzle_con_st *conn) {
  assert(conn);

  drizzle_return_t ret;
  drizzle_result_st result;

  drizzle_query_str(conn, &result, SKY_DB_USE, &ret);

  if (ret != DRIZZLE_RETURN_OK) {
    return false;
  }
  drizzle_result_free(&result);
  return true;
}

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
  drizzle_query_str(&connection, &result, SKY_DB_DROP, &ret);

  if (ret != DRIZZLE_RETURN_OK) {
    report_error(drizzle_con_error(&connection));
    drizzle_free(&drizzle);
    return false;
  }
  drizzle_result_free(&result);

  /* Now we actually create the database */
  drizzle_query_str(&connection, &result, SKY_DB_CREATE, &ret);

  if (ret != DRIZZLE_RETURN_OK) {
    report_error(drizzle_con_error(&connection));
    drizzle_free(&drizzle);
    return false;
  }
  drizzle_result_free(&result);

  /* Switch to the database that we want to work in */
  if (switch_to_skyload_database(&connection) == false) {
    report_error(drizzle_con_error(&connection));
    drizzle_free(&drizzle);
    return false;
  }

  /* Create the test table */
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

static bool drop_skyload_database(SKY_SHARE *share) {
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

  drizzle_query_str(&connection, &result, SKY_DB_DROP, &ret);

  if (ret != DRIZZLE_RETURN_OK) {
    report_error(drizzle_con_error(&connection));
    return false;
  }

  drizzle_result_free(&result);

  sky_close_connection(&connection);
  drizzle_free(&drizzle);
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

  fprintf(stderr, "Connection Created. Thread: %d\n", context->unique_id);

  /* Switch to the test database */
  if (switch_to_skyload_database(&context->connection) == false) {
    report_error(drizzle_con_error(&context->connection));
    context->aborted = true;
    drizzle_free(&context->database_handle);
    pthread_exit(NULL);
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
  if (handle_options(share, argc, argv) == false)
    return EXIT_FAILURE;

  /* Check if the provided options make sense */
  if (check_options(share) == false)
    return EXIT_FAILURE;

  /* Create worker object(s) */
  if ((workers = create_workers(share)) == NULL) {
    report_error("out of memory");
    return EXIT_FAILURE;
  }

  /* creates a database for skyload to play in */
  if (create_skyload_database(share) == false)
    return EXIT_FAILURE;

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

  /* skyload is done, drop the database unless specified not to */
  if (share->keep_db == false) {
    if (drop_skyload_database(share) == false)
      return EXIT_FAILURE;
  }

  destroy_workers(workers);
  sky_share_free(share);
  return EXIT_SUCCESS;
}
