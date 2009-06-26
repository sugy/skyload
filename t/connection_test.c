/* 
 * Copyright (C) 2009 Toru Maesaka <dev@torum.net>
 * All Rights Reserved.
 *
 * Use and distribution of this program is licensed under the
 * BSD license. See the COPYING file for full text.
 */

#include "../skyfall.h"

static bool connection_init_test(void);

int main(void) {
  if (connection_init_test() == false)
    return EXIT_FAILURE;

  return EXIT_SUCCESS;
}

static bool connection_init_test(void) {
  SKYFALL_SHARE *share;
  drizzle_st drizzle;
  drizzle_con_st connection;
  bool rv;

  drizzle_create(&drizzle);

  if (&drizzle == NULL)
    return false;

  if ((share = skyfall_share_new()) == NULL)
    return false;

  /* this should not return true since the 'share' object does
     not have sufficient information to create a connection
     structure */
  rv = skyfall_create_connection(share, &drizzle, &connection);

  if (rv) {
    drizzle_free(&drizzle);
    return false;
  }

  if ((share->server = strdup("localhost")) == NULL)
    return false;

  /* this should still fail since the hostname by itself
     is not sufficient to create a connection structure */
  rv = skyfall_create_connection(share, &drizzle, &connection);

  if (rv) {
    drizzle_free(&drizzle);
    return false;
  }

  share->port = (in_port_t)4427; 

  /* there is enough information now so rv should be set to true */
  rv = skyfall_create_connection(share, &drizzle, &connection);

  if (rv == false) {
    drizzle_free(&drizzle);
    return false;
  }

  skyfall_close_connection(&connection);

  drizzle_free(&drizzle);
  skyfall_share_free(share);
  return true;
}
