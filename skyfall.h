/* 
 * Copyright (C) 2009 Toru Maesaka <dev@torum.net>
 * All Rights Reserved.
 *
 * Use and distribution of this program is licensed under the
 * BSD license. See the COPYING file for full text.
 */

#ifndef __SKYFALL_H__
#define __SKYFALL_H__

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

#define SKYFALL_DB_NAME   "skyfall"
#define SKYFALL_DB_CREATE "CREATE DATABASE skyfall"
#define SKYFALL_DB_DROP   "DROP DATABASE IF EXISTS skyfall"
#define SKYFALL_DB_USE    "USE skyfall"

#define SKYFALL_MAX_COLS  128
#define SKYFALL_STRSIZ    1024
#define SKYFALL_RAND_SEED 149
 
/* This enumerator helps the query generator decide what
   kind of test data it should generate for a given column */
typedef enum {
  COLUMN_SEQUENTIAL,
  COLUMN_RANDOM,
  COLUMN_VARIABLE
} SKYFALL_COLUMN_TYPE;

/* Singly linked list node to help the query generator. This
   object belongs in the shared structure (below) and should
   not be altered at runtime */
typedef struct _column {
  SKYFALL_COLUMN_TYPE type; /* Type of the data to generate */
  struct _column *next;     /* Pointer to the next column if any */
  size_t length;            /* Length of the data to generate */
} SKYFALL_COLUMN_NODE;

/* Minimum spec linked list structure for representing table
   columns. By meaning 'minimum' only push() and free_all()
   interfaces are provided for this list */
typedef struct {
  SKYFALL_COLUMN_NODE *head;
  SKYFALL_COLUMN_NODE *tail;
  size_t size;
} SKYFALL_COLUMN_LIST;

/* Object shared among all worker threads. Only add items that
   will not be updated at runtime to this struct  */
typedef struct {
  SKYFALL_COLUMN_LIST *column_list;
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
} SKYFALL_SHARE;
 
typedef struct {
  SKYFALL_SHARE *share;
  pthread_t thread_id;
  drizzle_st database_handle;
  drizzle_con_st connection;
  bool aborted;
  uint32_t unique_id;
  uint32_t current_seq_id[SKYFALL_MAX_COLS];
} SKYFALL_WORKER;

/* allocator and deallocator. don't add anything more than
   memory allocation and basic variable initialization */
SKYFALL_WORKER *skyfall_worker_new(void);
void skyfall_worker_free(SKYFALL_WORKER *worker);

SKYFALL_SHARE *skyfall_share_new(void);
void skyfall_share_free(SKYFALL_SHARE *share);

/* takes care of getopt_long() handling */
bool handle_options(SKYFALL_SHARE *share, int argc, char **argv);

/* checks if the user options make sense */
bool check_options(SKYFALL_SHARE *share);

/* checks the number of SQL statement occurrences in the haystack */
uint32_t string_occurrence(const char *haystack, const char *needle);

/* in-house implementation of tolower(3) for compatibility issues */
char *skyfall_tolower(char *string);

/* initialize a connection for the given database handle */
bool skyfall_create_connection(SKYFALL_SHARE *share, drizzle_st *handle,
                               drizzle_con_st *conn);

/* closes then frees a connection */
void skyfall_close_connection(drizzle_con_st *conn);

/* create an array of workers*/
SKYFALL_WORKER **create_workers(SKYFALL_SHARE *share);

/* free an array of workers*/
void destroy_workers(SKYFALL_WORKER **workers);

/* create a new column list */
SKYFALL_COLUMN_LIST *skyfall_column_list_new();

/* free column list */
void skyfall_column_list_free(SKYFALL_COLUMN_LIST *list);

/* create a node for the column list */
SKYFALL_COLUMN_NODE *skyfall_column_node_new(void);

/* push a column meta information node to the list */
bool skyfall_column_list_push(SKYFALL_COLUMN_LIST *head,
                              SKYFALL_COLUMN_TYPE type,
                              const size_t length);

/* delete the entire column list */
void skyfall_column_free_all(SKYFALL_COLUMN_LIST *list);

/* calculates time difference in microseconds */
uint64_t timediff(struct timeval from, struct timeval to);

/* prints the usage of this program */
void usage(void);

/* used for consistent error output */
void report_error(const char *error);

#endif
