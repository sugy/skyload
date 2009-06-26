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
  share->create_query = NULL;
  share->select_query = NULL;
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

  free(share);
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

void usage() {
  printf("Skyfall: Parameters with '=' requires an argument\n");
  printf("  --server=      : Server Hostname (required)\n");
  printf("  --port=        : Server Port\n");
  printf("  --table=       : Table Creation Statement (required)\n");
  printf("  --select=      : Select Statement\n");
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
