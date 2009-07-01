/* 
 * Copyright (C) 2009 Toru Maesaka <dev@torum.net>
 * All Rights Reserved.
 *
 * Use and distribution of this program is licensed under the
 * BSD license. See the COPYING file for full text.
 */

#include "skyfall.h"

SKYFALL_WORKER *skyfall_worker_new(void) {
  SKYFALL_WORKER *worker = malloc(sizeof(*worker));

  if (worker == NULL) {
    return NULL;
  }
  worker->aborted = false;
  worker->share = NULL;
  worker->unique_id = 0;
  return worker;
}

void skyfall_worker_free(SKYFALL_WORKER *worker) {
  if (worker != NULL)
    free(worker);
}

SKYFALL_SHARE *skyfall_share_new(void) {
  SKYFALL_SHARE *share = malloc(sizeof(*share));

  if (share == NULL) {
    return NULL;
  }

  share->server = NULL;
  share->port = DRIZZLE_DEFAULT_PORT;
  share->column_list = NULL;
  share->create_query = NULL;
  share->select_query = NULL;
  share->insert_tmpl = NULL;
  share->nwrite = 0;
  share->nread = 0;
  share->concurrency = 1;
  share->protocol = 0;

  return share;
}

void skyfall_share_free(SKYFALL_SHARE *share) {
  assert(share);

  if (share->server != NULL)
    free(share->server);

  if (share->create_query != NULL)
    free(share->create_query);

  if (share->select_query != NULL)
    free(share->select_query);

  if (share->insert_tmpl != NULL)
    free(share->insert_tmpl);

  free(share);
}

bool skyfall_create_connection(SKYFALL_SHARE *share, drizzle_st *handle,
                               drizzle_con_st *conn) {
  assert(share && handle);

  if (share->server == NULL || share->port == 0)
    return false;

  if (drizzle_con_create(handle, conn) == NULL)
    return false;

  drizzle_con_set_tcp(conn, share->server, share->port);
  drizzle_con_add_options(conn, share->protocol);
  return true;
}

void skyfall_close_connection(drizzle_con_st *conn) {
  assert(conn);
  drizzle_con_close(conn);
  drizzle_con_free(conn);
}

SKYFALL_WORKER **create_workers(SKYFALL_SHARE *share) {
  assert(share && share->concurrency > 0);

  SKYFALL_WORKER **workers = malloc(sizeof(SKYFALL_WORKER *)
                                    * share->concurrency);
  if (workers == NULL)
    return NULL;

  for (int i = 0; i < share->concurrency; i++) {
    if ((workers[i] = skyfall_worker_new()) == NULL)
      return NULL;

    drizzle_create(&workers[i]->database_handle);
    workers[i]->share = share;
    workers[i]->unique_id = i + 1;
  }
  return workers;
}

void destroy_workers(SKYFALL_WORKER **workers) {
  assert(workers);

  uint32_t nworkers = workers[0]->share->concurrency;

  for (int i = 0; i < nworkers; i++) {
    drizzle_free(&workers[i]->database_handle);
    skyfall_worker_free(workers[i]);
  }
  free(workers);
}


SKYFALL_COLUMN_LIST *skyfall_column_list_new() {
  SKYFALL_COLUMN_LIST *list;

  if ((list = malloc(sizeof(*list))) == NULL)
    return NULL;

  list->head = NULL;
  list->tail = NULL;
  list->size = 0;
  return list;
}

void skyfall_column_list_free(SKYFALL_COLUMN_LIST *list) {
  assert(list);
  skyfall_column_free_all(list);
  free(list);
}

SKYFALL_COLUMN_NODE *skyfall_column_node_new() {
  SKYFALL_COLUMN_NODE *col = malloc(sizeof(*col));

  if (col == NULL)
    return NULL;

  col->type = COLUMN_RANDOM;
  col->length = 0;
  col->next = NULL;
  return col;
}

void skyfall_column_free_all(SKYFALL_COLUMN_LIST *list) {
  assert(list && list->head);

  SKYFALL_COLUMN_NODE *current = list->head;
  SKYFALL_COLUMN_NODE *temp;

  while (current != NULL) {
    temp = current;
    current = current->next;
    free(temp);
  }
}

bool skyfall_column_list_push(SKYFALL_COLUMN_LIST *list,
                              SKYFALL_COLUMN_TYPE type,
                              const size_t length) {
  assert(list);

  SKYFALL_COLUMN_NODE *col = skyfall_column_node_new();

  if (col == NULL)
    return false;

  col->type = type;
  col->length = length;
  list->size++;

  if (list->head == NULL) {
    list->head = col;
    list->tail = col;
    return true;
  }

  list->tail->next = col;
  list->tail = col;
  return true;
}

uint32_t string_occurrence(const char *haystack, const char *needle) {
  assert(haystack && needle);

  uint32_t count;
  const char *ptr = haystack;

  for (count = 0; (ptr = strstr(ptr, needle)) != NULL; count++)
    ptr++;

  return count;
}

char *skyfall_tolower(char *string) {
  assert(string);

  char *pos = string;
  while(*pos != '\0') {
    if(*pos >= 'A' && *pos <= 'Z')
      *pos += 'a' - 'A';
    pos++;
  }
  return string;
}

uint64_t timediff(struct timeval from, struct timeval to) {
  uint64_t us, s = 0;

  us = from.tv_usec - to.tv_usec;
  if ((s = from.tv_sec - to.tv_sec) > 0) {
    s *= 1000 * 1000;
  }
  return s + us;
}

void usage() {
  printf("Skyfall: Parameters with '=' requires an argument\n");
  printf("  --server=      : Server Hostname (required)\n");
  printf("  --port=        : Server Port\n");
  printf("  --table=       : Table Creation Statement (required)\n");
  printf("  --select=      : Select Statement\n");
  printf("  --insert=      : Insert Statement Template");
  printf("  --concurrency= : Number of simultaneous clients\n");
  printf("  --rows=        : Number of rows to insert into the table\n");
  printf("  --nread=       : Number of SELECT statement(s) to execute\n");
  printf("  --mysql        : Use MySQL Protocol\n");
  printf("  --help         : Print this help\n");
  exit(EXIT_SUCCESS);
}

void report_error(const char *error) {
  fprintf(stderr, "skyfall error: %s\n", error);
}
