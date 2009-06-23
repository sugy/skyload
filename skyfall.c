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

  skyfall->server = NULL;
  skyfall->create_query = NULL;
  skyfall->select_query = NULL;
  skyfall->mysql_protocol = false;
  skyfall->nwrite = 0;
  skyfall->nread = 0;

  return skyfall;
}

void skyfall_free(SKYFALL *skyfall) {
  assert(skyfall);

  if (skyfall->server != NULL)
    free(skyfall->server);

  if (skyfall->create_query != NULL)
    free(skyfall->create_query);

  if (skyfall->select_query != NULL)
    free(skyfall->select_query);

  free(skyfall);
}

void usage() {
  printf("Skyfall: Parameters with '=' requires an argument\n");
  printf("  --server= : Server Hostname (required)\n");
  printf("  --port=   : Server Port\n");
  printf("  --table=  : Table Creation Statement (required)\n");
  printf("  --select= : Select Statement\n");
  printf("  --nwrite= : Number of rows to insert into the table\n");
  printf("  --nread=  : Number of SELECT statement(s) to execute\n");
  printf("  --mysql   : Use MySQL Protocol\n");
  printf("  --help    : Print this help\n");
  exit(EXIT_SUCCESS);
}

void report_error(const char *error) {
  fprintf(stderr, "Skyfall Error: %s\n", error);
}

int main(int argc, char **argv) {
  SKYFALL *skyfall;

  if (argc == 1)
    usage();

  if ((skyfall = skyfall_new()) == NULL) {
    report_error("out of memory");
    return EXIT_FAILURE;
  }

  if (handle_options(skyfall, argc, argv) == false)
    return EXIT_FAILURE;

  if (check_options(skyfall) == false)
    return EXIT_FAILURE;

  skyfall_free(skyfall);
  return EXIT_SUCCESS;
}
