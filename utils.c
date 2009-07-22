/* 
 * Copyright (C) 2009 Toru Maesaka <dev@torum.net>
 * All Rights Reserved.
 *
 * Use and distribution of this program is licensed under the
 * BSD license. See the COPYING file for full text.
 */

#include "skyload.h"

SKY_WORKER *sky_worker_new(void) {
  SKY_WORKER *worker = malloc(sizeof(*worker));

  if (worker == NULL) {
    return NULL;
  }
  worker->aborted = false;
  worker->share = NULL;
  worker->unique_id = 0;
  worker->total_insert_time = 0;
  return worker;
}

void sky_worker_free(SKY_WORKER *worker) {
  if (worker != NULL)
    free(worker);
}

SKY_SHARE *sky_share_new(void) {
  SKY_SHARE *share = malloc(sizeof(*share));

  if (share == NULL) {
    return NULL;
  }

  share->server = NULL;
  share->query_list = NULL;
  share->create_query = NULL;
  share->select_query = NULL;
  share->insert_tmpl = NULL;
  share->keep_db = false;
  share->port = 0;
  share->nwrite = 0;
  share->nread = 0;
  share->concurrency = 1;
  share->protocol = 0;

  return share;
}

void sky_share_free(SKY_SHARE *share) {
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

SKY_LIST *sky_list_new(void) {
  SKY_LIST *list;

  if ((list = malloc(sizeof(*list))) == NULL)
    return NULL;

  list->head = NULL;
  list->tail = NULL;
  list->size = 0;
  return list;
}

void sky_list_free(SKY_LIST *list) {
  if (list == NULL) return;

  SKY_LIST_NODE *current = list->head;
  SKY_LIST_NODE *temp = NULL;

  /* free all linked nodes */
  while(current != NULL) {
    temp = current;
    current = current->next;
    sky_node_free(temp);
  }
  free(list);
}

SKY_LIST_NODE *sky_node_new(void) {
  SKY_LIST_NODE *node;

  if ((node = malloc(sizeof(*node))) == NULL)
    return NULL;

  node->next = NULL;
  node->data = NULL;
  node->length = 0;
  return node;
}

void sky_node_free(SKY_LIST_NODE *node) {
  if (node == NULL) return;
  free(node->data);
  free(node);
}

bool sky_list_push(SKY_LIST *list, const char *value, const size_t length) {
  assert(list && length > 0);

  SKY_LIST_NODE *node; 

  if ((node = sky_node_new()) == NULL)
    return false;

  node->data = malloc(length);
  node->length = length;
  strncpy(node->data, value, length);
  list->size++;

  if (list->head == NULL) {
    list->head = node;
    list->tail = node;
    return true;
  }

  list->tail->next = node;
  list->tail = node;
  return true;
}

bool sky_create_connection(SKY_SHARE *share, drizzle_st *handle,
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

void sky_close_connection(drizzle_con_st *conn) {
  assert(conn);
  drizzle_con_close(conn);
  drizzle_con_free(conn);
}

SKY_WORKER **create_workers(SKY_SHARE *share) {
  assert(share && share->concurrency > 0);

  SKY_WORKER **workers = malloc(sizeof(SKY_WORKER *) * share->concurrency);

  if (workers == NULL)
    return NULL;

  /* seed the random number generator beforehand. */
  srandom(SKY_RAND_SEED);

  for (int i = 0; i < share->concurrency; i++) {
    if ((workers[i] = sky_worker_new()) == NULL)
      return NULL;

    drizzle_create(&workers[i]->database_handle);
    workers[i]->share = share;
    workers[i]->unique_id = i + 1;

    for (int j = 0; j < SKY_MAX_COLS; j++)
      workers[i]->current_seq_id[j] = workers[i]->unique_id;
  }
  return workers;
}

void destroy_workers(SKY_WORKER **workers) {
  assert(workers);

  uint32_t nworkers = workers[0]->share->concurrency;

  for (int i = 0; i < nworkers; i++) {
    drizzle_free(&workers[i]->database_handle);
    sky_worker_free(workers[i]);
  }
  free(workers);
}

uint32_t string_occurrence(const char *haystack, const char *needle) {
  assert(haystack && needle);

  uint32_t count;
  const char *ptr = haystack;

  for (count = 0; (ptr = strstr(ptr, needle)) != NULL; count++)
    ptr++;

  return count;
}

char *sky_tolower(char *string) {
  assert(string);

  char *pos = string;
  while (*pos != '\0') {
    if (*pos >= 'A' && *pos <= 'Z')
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

uint32_t rows_to_write(SKY_WORKER *worker){
  assert(worker);

  uint32_t count = worker->share->nwrite / worker->share->concurrency;

  if (worker->unique_id == worker->share->concurrency)
    count += worker->share->nwrite % worker->share->concurrency;

  return count;
}

void aggregate_worker_result(SKY_WORKER **workers) {
  double data_load_time = 0;
  bool aborted = false;

  for (int i = 0; i < workers[0]->share->concurrency; i++) {
    if (workers[i]->aborted) {
      aborted = true;
      break;
    }

    data_load_time += workers[i]->total_insert_time;
    data_load_time /= 1000000;
  }

  printf("\n");

  if (aborted) {
    report_error("failed to run load test");
  } else {
    printf("Concurrent Connections : %d\n", workers[0]->share->concurrency);
    printf("Total Time to INSERT   : %.5lf secs\n", data_load_time);
    printf("Rows Loaded            : %d\n", workers[0]->share->nwrite);
  }
}

void usage() {
  printf("skyload: Parameters with '=' requires an argument\n");
  printf("  --server=      : Server Hostname (required)\n");
  printf("  --port=        : Server Port\n");
  printf("  --table=       : Table Creation Statement (required)\n");
  printf("  --insert=      : Insert Statement Template\n");
  printf("  --concurrency= : Number of simultaneous clients\n");
  printf("  --rows=        : Number of rows to insert into the table\n");
  printf("  --keep         : Don't delete the database after the test\n");
  printf("  --mysql        : Use MySQL Protocol\n");
  printf("  --help         : Print this help\n");
  exit(EXIT_SUCCESS);
}

void report_error(const char *error) {
  fprintf(stderr, "skyload error: %s\n", error);
}
