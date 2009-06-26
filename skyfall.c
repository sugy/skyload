/* 
 * Copyright (C) 2009 Toru Maesaka <dev@torum.net>
 * All Rights Reserved.
 *
 * Use and distribution of this program is licensed under the
 * BSD license. See the COPYING file for full text.
 */

#include "skyfall.h"

static bool create_skyfall_database(SKYFALL_SHARE *share) {
  assert(share);

  drizzle_st drizzle;
  drizzle_con_st connection;
  drizzle_return_t ret;
  drizzle_result_st result;

  drizzle_create(&drizzle);
  
  if (drizzle_con_create(&drizzle, &connection) == NULL) {
    drizzle_free(&drizzle);
    return false;
  }

  drizzle_con_set_tcp(&connection, share->server, share->port);
  drizzle_con_add_options(&connection, share->protocol);

  /* Attempt to drop the database just in case the database
     still exists from the previous run. */
  drizzle_query_str(&connection, &result, SKYFALL_DB_DROP, &ret);

  if (ret != DRIZZLE_RETURN_OK) {
    report_error(drizzle_con_error(&connection));
    return false;
  }

  drizzle_result_free(&result);

  /* Now we actually create the database */
  drizzle_query_str(&connection, &result, SKYFALL_DB_CREATE, &ret);

  if (ret != DRIZZLE_RETURN_OK) {
    report_error(drizzle_con_error(&connection));
    return false;
  }

  drizzle_result_free(&result);

  drizzle_con_close(&connection);
  drizzle_con_free(&connection);
  drizzle_free(&drizzle);
  return true;
}

static bool drop_skyfall_database(SKYFALL_SHARE *share) {
  assert(share);

  drizzle_st drizzle;
  drizzle_con_st connection;
  drizzle_return_t ret;
  drizzle_result_st result;

  drizzle_create(&drizzle);
  
  if (drizzle_con_create(&drizzle, &connection) == NULL) {
    report_error(drizzle_con_error(&connection));
    drizzle_free(&drizzle);
    return false;
  }

  drizzle_con_set_tcp(&connection, share->server, share->port);
  drizzle_con_add_options(&connection, share->protocol);

  drizzle_query_str(&connection, &result, SKYFALL_DB_DROP, &ret);

  if (ret != DRIZZLE_RETURN_OK) {
    report_error(drizzle_con_error(&connection));
    return false;
  }

  drizzle_result_free(&result);

  drizzle_con_close(&connection);
  drizzle_con_free(&connection);
  drizzle_free(&drizzle);
  return true;
}

void *workload(void *arg) {
  assert(arg);
  SKYFALL_WORKER *context = (SKYFALL_WORKER *)arg;
  fprintf(stderr, "I'm your thread! id: %d\n", context->unique_id);
  return NULL;
}

int main(int argc, char **argv) {
  SKYFALL_SHARE *share;
  SKYFALL_WORKER **workers;
  pthread_attr_t joinable;

  if (argc == 1)
    usage();

  /* This object is shared among all worker threads */
  if ((share = skyfall_share_new()) == NULL) {
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

  /* creates a database for skyfall to play in */
  if (create_skyfall_database(share) == false)
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

  /* skyfall is done, drop the database */
  if (drop_skyfall_database(share) == false)
    return EXIT_FAILURE;

  destroy_workers(workers);
  skyfall_share_free(share);
  return EXIT_SUCCESS;
}
