/* 
 * Copyright (C) 2009 Toru Maesaka <dev@torum.net>
 * All Rights Reserved.
 *
 * Use and distribution of this program is licensed under the
 * BSD license. See the COPYING file for full text.
 */

#include "skyfall.h"

typedef enum {
  OPT_HELP = 'h',
  OPT_PORT = 'p',
  OPT_SERVER = 's',
  OPT_CREATE_QUERY,
  OPT_SELECT_QUERY,
  OPT_NUM_ROWS,
  OPT_CONCURRENCY,
  OPT_NUM_SELECT,
  OPT_MYSQL_PROT
} skyfall_options;

static struct option longopts[] = {
  {"help", no_argument, NULL, OPT_HELP},
  {"mysql", no_argument, NULL, OPT_MYSQL_PROT},
  {"port", required_argument, NULL, OPT_PORT},
  {"server", required_argument, NULL, OPT_SERVER},
  {"table", required_argument, NULL, OPT_CREATE_QUERY},
  {"select", required_argument, NULL, OPT_SELECT_QUERY},
  {"rows", required_argument, NULL, OPT_NUM_ROWS},
  {"concurrency", required_argument, NULL, OPT_CONCURRENCY},
  {"nread", required_argument, NULL, OPT_NUM_SELECT},
  {0, 0, 0, 0}
};

bool check_options(SKYFALL_SHARE *share) {
  assert(share);
  bool rv = true;

  if (share->server == NULL) {
    report_error("hostname is missing");
    rv = false;
  }

  if (share->create_query == NULL) {
    report_error("table creation statement is missing");
    rv = false;
  }

  /* currently skyfall only supports one table */
  if (string_occurrence(share->create_query, "CREATE TABLE") > 1 ||
      string_occurrence(share->create_query, "create table") > 1) {
    report_error("only one table can be created");
    rv = false;
  }

  if (share->nwrite <= 0) {
    report_error("--rows must be set to greater than 0");
    rv = false;
  }
  return rv;
}

bool handle_options(SKYFALL_SHARE *share, int argc, char **argv) {
  assert(share);
  int ch, temp;

  while ((ch = getopt_long(argc, argv, "hs:p:", longopts, NULL)) != -1) {
    switch(ch) {
    case OPT_HELP:
      usage();
      break;
    case OPT_SERVER:
      if ((share->server = strdup(optarg)) == NULL) {
        report_error("out of memory");
        return false;
      }
      break;
    case OPT_CREATE_QUERY:
      if ((share->create_query = strdup(optarg)) == NULL) {
        report_error("out of memory");
        return false;
      }
      break;
    case OPT_SELECT_QUERY:
      if ((share->select_query = strdup(optarg)) == NULL) {
        report_error("out of memory");
        return false;
      }
      break;
    case OPT_PORT:
      share->port = (in_port_t)atoi(optarg);
      break;
    case OPT_NUM_ROWS:
      share->nwrite = (in_port_t)atoi(optarg);
      break;
    case OPT_CONCURRENCY:
      temp = atoi(optarg);
      share->concurrency = (in_port_t)(temp <= 0) ? 1 : temp;
      break;
    case OPT_NUM_SELECT:
      share->nread = (in_port_t)atoi(optarg);
      break;
    case OPT_MYSQL_PROT:
      share->protocol = DRIZZLE_CON_MYSQL;
      break;
    }
  }
  return true;
}
