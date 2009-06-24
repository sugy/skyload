/* 
 * Copyright (C) 2009 Toru Maesaka <dev@torum.net>
 * All Rights Reserved.
 *
 * Use and distribution of this program is licensed under the
 * BSD license. See the COPYING file for full text.
 */

#ifndef __SKYFALL_H__
#define __SKYFALL_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <assert.h>

#include <libdrizzle/drizzle_client.h>

#define SKYFALL_DB_NAME "skyfall"
#define SKYFALL_DB_CREATE "CREATE DATABASE skyfall"
#define SKYFALL_DB_DROP "DROP DATABASE IF EXISTS skyfall"

/* Object shared among all worker threads. Only add items that
   will not be updated at runtime to this struct  */
typedef struct {
  in_port_t port;
  char *server;
  char *create_query;
  char *select_query;
  uint16_t protocol;
  uint32_t nwrite;
  uint32_t nread;
} SKYFALL_SHARE;

typedef struct {
  SKYFALL_SHARE *share;
  drizzle_st database_handle;
} SKYFALL;

SKYFALL *skyfall_new(void);
void skyfall_free(SKYFALL *skyfall);

SKYFALL_SHARE *skyfall_share_new(void);
void skyfall_share_free(SKYFALL_SHARE *share);

/* takes care of getopt_long() handling */
bool handle_options(SKYFALL_SHARE *share, int argc, char **argv);

/* checks if the user options make sense */
bool check_options(SKYFALL_SHARE *share);

/* prints the usage of this program */
void usage(void);

/* used for consistent error output */
void report_error(const char *error);

#endif
