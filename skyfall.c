/* 
 * Copyright (C) 2009 Toru Maesaka <dev@torum.net>
 * All Rights Reserved.
 *
 * Use and distribution of this program is licensed under the
 * BSD license. See the COPYING file for full text.
 */

#include "skyfall.h"

static bool create_skyfall_database() {
  drizzle_st drizzle;
  drizzle_con_st connection;
  drizzle_return_t ret;
  drizzle_result_st result;

  drizzle_create(&drizzle);
  
  if (drizzle_con_create(&drizzle, &connection) == NULL) {
    drizzle_free(&drizzle);
    return false;
  }

  /* Attempt to drop the database just in case the database
     still exists from the previous run. */
  drizzle_query_str(&connection, &result, SKYFALL_DB_DROP, &ret);
  drizzle_result_free(&result);

  if (ret != DRIZZLE_RETURN_OK) {
    report_error(drizzle_con_error(&connection));
    return false;
  }

  /* Now we actually create the database */
  drizzle_query_str(&connection, &result, SKYFALL_DB_CREATE, &ret);
  drizzle_result_free(&result);

  if (ret != DRIZZLE_RETURN_OK) {
    report_error(drizzle_con_error(&connection));
    return false;
  }

  drizzle_con_close(&connection);
  drizzle_con_free(&connection);
  drizzle_free(&drizzle);
  return true;
}

static bool drop_skyfall_database() {
  drizzle_st drizzle;
  drizzle_con_st connection;
  drizzle_return_t ret;
  drizzle_result_st result;

  drizzle_create(&drizzle);
  
  if (drizzle_con_create(&drizzle, &connection) == NULL) {
    drizzle_free(&drizzle);
    return false;
  }

  drizzle_query_str(&connection, &result, SKYFALL_DB_DROP, &ret);
  drizzle_result_free(&result);

  if (ret != DRIZZLE_RETURN_OK) {
    report_error(drizzle_con_error(&connection));
    return false;
  }

  drizzle_con_close(&connection);
  drizzle_con_free(&connection);
  drizzle_free(&drizzle);
  return true;
}

int main(int argc, char **argv) {
  SKYFALL_SHARE *share;
  SKYFALL *skyfall;

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

  /* TODO: encapsulate this into create_workers() which will
           be capable of creating multiple workers */
  if ((skyfall = skyfall_new()) == NULL) {
    report_error("out of memory");
    return EXIT_FAILURE;
  }

  skyfall->share = share;

  drizzle_create(&skyfall->database_handle);

  /* creates a database for skyfall to play in */
  create_skyfall_database();

  /* skyfall is done, drop the database */
  drop_skyfall_database();

  skyfall_share_free(share);
  skyfall_free(skyfall);
  return EXIT_SUCCESS;
}
