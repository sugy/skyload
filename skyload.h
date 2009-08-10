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
#include <sys/time.h> 

#include <libdrizzle/drizzle_client.h>

#define DRIZZLE_DEFAULT_PORT 4427
#define MYSQL_DEFAULT_PORT 3306

#define SKY_DB_NAME   "skyload"
#define SKY_DB_CREATE "CREATE DATABASE skyload"
#define SKY_DB_DROP   "DROP DATABASE IF EXISTS skyload"

#define SKY_PLACEHOLDER_SYM '%'

#define SKY_STRSIZ    1024
#define SKY_MAX_COLS  128
#define SKY_RAND_SEED 149
 
/* Structure to represent a node for a singly linked query list */
typedef struct _sky_node {
  struct _sky_node *next;
  char *data;
  size_t length;
} SKY_LIST_NODE;

/* Structure to represent a singly linked list used to represent
   the external SQL file provided by the user. Only created when
   an external file is provided with the '--file=' option */
typedef struct {
  SKY_LIST_NODE *head;
  SKY_LIST_NODE *tail;
  size_t size;
} SKY_LIST;

/* Object shared among all worker threads. Only add items that
   will not be updated at runtime to this struct  */
typedef struct {
  SKY_LIST *load_queries; /* Singly linked list for external load queries */
  SKY_LIST *read_queries; /* Singly linked list for external read queries */
  in_port_t port;         /* DBMS port to talk to */
  char *server;           /* DBMS Hostname */
  char *database_name;    /* User specified database to run tests on */
  char *create_query;     /* CREATE TABLE query */
  char *insert_tmpl;      /* INSERT query template */
  char *load_file_path;   /* Path to the provided Load-SQL file */
  char *read_file_path;   /* Path to the provided Read-SQL file */
  bool keep_db;           /* Whether to drop the test database or not */
  uint16_t protocol;      /* Database protocol */
  uint16_t columns;       /* Number of columns in the table */
  uint32_t nwrite;        /* Number of rows to INSERT */
  uint32_t runs;          /* Number of times to run the test */
  uint32_t concurrency;   /* Number of concurrent connections */
  double file_load_time;  /* Time taken to process a load file */
} SKY_SHARE;
 
/* Structure to represent a worker. Number of workers created
   is relative to the specified concurrency level. */
typedef struct {
  SKY_SHARE *share;
  pthread_t thread_id;
  drizzle_st database_handle;
  drizzle_con_st connection;
  bool aborted;
  uint32_t unique_id;
  uint32_t current_seq_id[SKY_MAX_COLS];
  uint64_t total_insert_time;
  uint64_t file_benchmark_time;
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

/* create a linked list */
SKY_LIST *sky_list_new(void);

/* free a linked list */
void sky_list_free(SKY_LIST *list);

/* create a node for the linked list */
SKY_LIST_NODE *sky_node_new(void);

/* push data to the list */
bool sky_list_push(SKY_LIST *list, const char *value, const size_t length);

/* free a linked list node */
void sky_node_free(SKY_LIST_NODE *node);

/* calculates time difference in microseconds */
uint64_t timediff(struct timeval from, struct timeval to);

/* caluclates the number of insertions that a given worker
   thread must perform */
uint32_t rows_to_write(SKY_WORKER *worker);

/* switch to the specified (or default) database */
bool switch_database(SKY_SHARE *share, drizzle_con_st *conn);

/* drop a database */
bool drop_database(SKY_SHARE *share);

/* populate the database based on the provided load-file */
bool preload_database(SKY_SHARE *share);

/* Aggregate and print the benchmark result held by all workers */ 
void aggregate_worker_result(SKY_WORKER **workers);

/* prints the usage of this program */
void usage(void);

/* used for consistent error output */
void report_error(const char *error);

#endif
