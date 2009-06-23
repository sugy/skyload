/* 
 * Copyright (C) 2009 Toru Maesaka <dev@torum.net>
 * All Rights Reserved.
 *
 * Use and distribution of this program is licensed under the
 * BSD license. See the COPYING file for full text.
 */

#include "skyfall.h"

SKYFALL *skyfall_new(void) {
  SKYFALL *skyfall = malloc(sizeof(*skyfall));

  if (skyfall == NULL) {
    return NULL;
  }

  skyfall->share = NULL;
  return skyfall;
}

void skyfall_free(SKYFALL *skyfall) {
  assert(skyfall);
  free(skyfall);
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
  share->protocol = 0;
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

void usage() {
  printf("Skyfall: Parameters with '=' requires an argument\n");
  printf("  --server= : Server Hostname (required)\n");
  printf("  --port=   : Server Port\n");
  printf("  --table=  : Table Creation Statement (required)\n");
  printf("  --select= : Select Statement\n");
  printf("  --rows=   : Number of rows to insert into the table\n");
  printf("  --nread=  : Number of SELECT statement(s) to execute\n");
  printf("  --mysql   : Use MySQL Protocol\n");
  printf("  --help    : Print this help\n");
  exit(EXIT_SUCCESS);
}

void report_error(const char *error) {
  fprintf(stderr, "skyfall error: %s\n", error);
}
