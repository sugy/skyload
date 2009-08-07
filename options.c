/* 
 * Copyright (C) 2009 Toru Maesaka <dev@torum.net>
 * All Rights Reserved.
 *
 * Use and distribution of this program is licensed under the
 * BSD license. See the COPYING file for full text.
 */

#include "skyload.h"

typedef enum {
  OPT_HELP = 'h',
  OPT_PORT = 'p',
  OPT_SERVER = 's',
  OPT_CREATE_QUERY,
  OPT_INSERT_TMPL,
  OPT_NUM_ROWS,
  OPT_CONCURRENCY,
  OPT_NUM_SELECT,
  OPT_KEEP_DB,
  OPT_USE_DB,
  OPT_LOAD_FILE,
  OPT_READ_FILE,
  OPT_NUM_RUNS,
  OPT_MYSQL_PROT
} sky_options;

static struct option longopts[] = {
  {"help", no_argument, NULL, OPT_HELP},
  {"keep", no_argument, NULL, OPT_KEEP_DB},
  {"mysql", no_argument, NULL, OPT_MYSQL_PROT},
  {"db", required_argument, NULL, OPT_USE_DB},
  {"port", required_argument, NULL, OPT_PORT},
  {"server", required_argument, NULL, OPT_SERVER},
  {"load-file", required_argument, NULL, OPT_LOAD_FILE},
  {"read-file", required_argument, NULL, OPT_READ_FILE},
  {"runs", required_argument, NULL, OPT_NUM_RUNS},
  {"table", required_argument, NULL, OPT_CREATE_QUERY},
  {"insert", required_argument, NULL, OPT_INSERT_TMPL},
  {"rows", required_argument, NULL, OPT_NUM_ROWS},
  {"concurrency", required_argument, NULL, OPT_CONCURRENCY},
  {0, 0, 0, 0}
};

bool check_options(SKY_SHARE *share) {
  assert(share);
  bool rv = true;

  if (share->server == NULL) {
    report_error("hostname is missing");
    rv = false;
  }

  /* skyload does not allow any write operations on the user
     supplied database. this policy is placed to avoid undesired
     updates on the database */
  if (share->database_name && (share->load_file_path || share->insert_tmpl)) {
    report_error("write operations on existing database is not allowed");
    return false;
  }

  /* skyload does not allow both INSERT template and load-file to
     be provided at the same time. return immediately since this
     is a critical rule */
  if (share->insert_tmpl && share->load_file_path) {
    report_error("INSERT template and load file cannot be supplied together");
    return false;
  }

  /* TODO: Check if there's a CREATE statement in the load file */
  if (!share->database_name) {
    if (!share->create_query && !share->load_file_path) {
      report_error("table creation statement or load-file is missing");
      return false;
    }
  }

  /* User had specified skyload to auto-generate data. In this
     case, skyload only supports one table */
  if (share->insert_tmpl) {
    if (string_occurrence(share->create_query, "create table") > 1) {
      report_error("only one table can be created");
      rv = false;
    }

    /* Check INSERT template validity */
    if (share->columns > SKY_MAX_COLS) {
      report_error("too many columns");
      rv = false;
    } else if (share->columns <= 0) {
      report_error("column placeholder is missing from the INSERT template");
      rv = false;
    }

    if (share->nwrite < 1) {
      report_error("--rows must be set to greater than 0");
      rv = false;
    }
  } else if (!share->load_file_path) {
    report_error("please supply INSERT template or load file");
    rv = false;
  }

  /* User had specified to provide their own read test */
  if (share->read_file_path) {
    if (share->runs < 1) {
      report_error("--runs must be set to greater than 0");
      rv = false;
    }
  }

  return rv;
}

bool handle_options(SKY_SHARE *share, int argc, char **argv) {
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
      sky_tolower(share->create_query);
      break;
    case OPT_INSERT_TMPL:
      if ((share->insert_tmpl = strdup(optarg)) == NULL) {
        report_error("out of memory");
        return false;
      }
      sky_tolower(share->insert_tmpl);
      share->columns = string_occurrence(share->insert_tmpl, "%");
      break;
    case OPT_LOAD_FILE:
      if ((share->load_file_path = strdup(optarg)) == NULL) {
        report_error("out of memory");
        return false;
      }
      break;
    case OPT_READ_FILE:
      if ((share->read_file_path = strdup(optarg)) == NULL) {
        report_error("out of memory");
        return false;
      }
      break;
    case OPT_USE_DB:
      if ((share->database_name = strdup(optarg)) == NULL) {
        report_error("out of memory");
        return false;
      }
      break;
    case OPT_NUM_RUNS:
      share->runs = (uint32_t)atoi(optarg);
      break;
    case OPT_PORT:
      share->port = (in_port_t)atoi(optarg);
      break;
    case OPT_NUM_ROWS:
      share->nwrite = (uint32_t)atoi(optarg);
      break;
    case OPT_CONCURRENCY:
      temp = atoi(optarg);
      share->concurrency = (temp <= 0) ? 1 : temp;
      break;
    case OPT_MYSQL_PROT:
      share->protocol = DRIZZLE_CON_MYSQL;
      break;
    case OPT_KEEP_DB:
      share->keep_db = true;
      break;
    default:
      usage();
      break;
    }
  }
  return true;
}
