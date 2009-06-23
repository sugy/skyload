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
  OPT_UPDATE_QUERY,
  OPT_SELECT_QUERY,
  OPT_NUM_ROWS,
  OPT_NUM_SELECT,
  OPT_MYSQL_PROT
} skyfall_options;

static struct option longopts[]= {
  {"help", no_argument, NULL, OPT_HELP},
  {"mysql", no_argument, NULL, OPT_MYSQL_PROT},
  {"port", required_argument, NULL, OPT_PORT},
  {"server", required_argument, NULL, OPT_SERVER},
  {"table", required_argument, NULL, OPT_CREATE_QUERY},
  {"select", required_argument, NULL, OPT_SELECT_QUERY},
  {"nwrite", required_argument, NULL, OPT_NUM_SELECT},
  {"nread", required_argument, NULL, OPT_NUM_ROWS},
  {0, 0, 0, 0}
};

bool check_options(SKYFALL *skyfall) {
  assert(skyfall);
  bool rv = true;
  bool query_existence = false;

  if (skyfall->server == NULL) {
    report_error("hostname is missing");
    rv = false;
  }

  if (skyfall->create_query == NULL) {
    report_error("table creation statement is missing");
    rv = false;
  }
  return rv;
}

bool handle_options(SKYFALL *skyfall, int argc, char **argv) {
  assert(skyfall);
  int ch;

  while ((ch = getopt_long(argc, argv, "hs:p:", longopts, NULL)) != -1) {
    switch(ch) {
    case OPT_HELP:
      usage();
      break;
    case OPT_SERVER:
      if ((skyfall->server = strdup(optarg)) == NULL) {
        report_error("out of memory");
        return false;
      }
      break;
    case OPT_CREATE_QUERY:
      if ((skyfall->create_query = strdup(optarg)) == NULL) {
        report_error("out of memory");
        return false;
      }
      break;
    case OPT_SELECT_QUERY:
      if ((skyfall->select_query = strdup(optarg)) == NULL) {
        report_error("out of memory");
        return false;
      }
      break;
    case OPT_PORT:
      skyfall->port = (in_port_t)atoi(optarg);
      break;
    case OPT_NUM_ROWS:
      skyfall->nwrite = (in_port_t)atoi(optarg);
      break;
    case OPT_NUM_SELECT:
      skyfall->nread = (in_port_t)atoi(optarg);
      break;
    case OPT_MYSQL_PROT:
      skyfall->mysql_protocol = true;
      break;
    }
  }
  return true;
}
