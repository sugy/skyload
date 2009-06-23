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

typedef struct {
  drizzle_st database_handle;
  in_port_t port;
  char *server;
  char *create_query;
  char *select_query;
  bool mysql_protocol;
  uint32_t nwrite;
  uint32_t nread;
} SKYFALL;

/* takes care of getopt_long() handling */
bool handle_options(SKYFALL *skyfall, int argc, char **argv);

/* prints the usage of this program */
void usage(void);

/* used for consistent error output */
void report_error(const char *error);

#endif
