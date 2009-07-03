/* 
 * Copyright (C) 2009 Toru Maesaka <dev@torum.net>
 * All Rights Reserved.
 *
 * Use and distribution of this program is licensed under the
 * BSD license. See the COPYING file for full text.
 */

#ifndef __SKYLOAD_H__
#define __SKYLOAD_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <getopt.h>
#include <assert.h>

#include <libdrizzle/drizzle_client.h>

#define DRIZZLE_DEFAULT_PORT 4427

#define SKY_DB_NAME   "skyload"
#define SKY_DB_CREATE "CREATE DATABASE skyload"
#define SKY_DB_DROP   "DROP DATABASE IF EXISTS skyload"
#define SKY_DB_USE    "USE skyload"

#define SKY_PLACEHOLDER_SYM '%'

#define SKY_MAX_COLS  128
#define SKY_STRSIZ    1024
#define SKY_RAND_SEED 149
 
/* Object shared among all worker threads. Only add items that
   will not be updated at runtime to this struct  */
typedef struct {
  in_port_t port;       /* DBMS port to talk to */
  char *server;         /* DBMS Hostname */
  char *create_query;   /* CREATE TABLE query */
  char *select_query;   /* SELECT query */
  char *insert_tmpl;    /* INSERT query template */
  uint16_t protocol;    /* Database protocol */
  uint16_t columns;     /* Number of columns in the table */
  uint32_t nwrite;      /* Number of rows to INSERT */
  uint32_t nread;       /* Number of times to SELECT */
  uint32_t concurrency; /* Number of concurrent connections */
} SKY_SHARE;
 
typedef struct {
  SKY_SHARE *share;
  pthread_t thread_id;
  drizzle_st database_handle;
  drizzle_con_st connection;
  bool aborted;
  uint32_t unique_id;
  uint32_t current_seq_id[SKY_MAX_COLS];
} SKY_WORKER;

/* allocator and deallocator. don't add anything more than
   memory allocation and basic variable initialization */
SKY_WORKER *sky_worker_new(void);
void sky_worker_free(SKY_WORKER *worker);

SKY_SHARE *sky_share_new(void);
void sky_share_free(SKY_SHARE *share);

/* takes care of getopt_long() handling */
bool handle_options(SKY_SHARE *share, int argc, char **argv);

/* checks if the user options make sense */
bool check_options(SKY_SHARE *share);

/* checks the number of SQL statement occurrences in the haystack */
uint32_t string_occurrence(const char *haystack, const char *needle);

/* in-house implementation of tolower(3) for compatibility issues */
char *sky_tolower(char *string);

/* initialize a connection for the given database handle */
bool sky_create_connection(SKY_SHARE *share, drizzle_st *handle,
                           drizzle_con_st *conn);

/* closes then frees a connection */
void sky_close_connection(drizzle_con_st *conn);

/* create an array of workers*/
SKY_WORKER **create_workers(SKY_SHARE *share);

/* free an array of workers*/
void destroy_workers(SKY_WORKER **workers);

/* calculates time difference in microseconds */
uint64_t timediff(struct timeval from, struct timeval to);

/* prints the usage of this program */
void usage(void);

/* used for consistent error output */
void report_error(const char *error);

#endif
