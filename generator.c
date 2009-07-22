/* 
 * Copyright (C) 2009 Toru Maesaka <dev@torum.net>
 * All Rights Reserved.
 *
 * Use and distribution of this program is licensed under the
 * BSD license. See the COPYING file for full text.
 */

#include "generator.h"

static uint32_t next_id(SKY_WORKER *worker, uint32_t col_num) {
  assert(worker);
  return worker->current_seq_id[col_num] += worker->share->concurrency;
}

size_t next_insert_query(SKY_WORKER *worker, char *buffer, size_t buflen) {
  size_t query_length;  
  char *pos, *write_ptr;

  /* copy everything up to the values. e.g. 'INSERT INTO t1 VALUES (' */
  write_ptr = buffer;
  pos = strchr(worker->share->insert_tmpl, SKY_PLACEHOLDER_SYM);
  query_length = 0;

  if (pos == NULL)
    return 0;

  size_t temp = pos - worker->share->insert_tmpl;
  size_t free_space = buflen - temp;

  if (temp > SKY_STRSIZ) {
    report_error("supplied INSERT query template is too long");
    return 0;
  }

  strncpy(write_ptr, worker->share->insert_tmpl, temp);
  write_ptr += temp;
  query_length += temp;

  /* rewind 1 byte so that the loop below can find the first placeholder */
  pos--; 

  /* now we look for placeholders and generate values for it */
  for (int i = 0; i < worker->share->columns; i++) {
    pos = strchr(pos, SKY_PLACEHOLDER_SYM);

    if (pos == NULL) {
      report_error("invalid INSERT query template");
      return 0;
    }

    if (strncmp(pos, PLACEHOLDER_RAND, PLACEHOLDER_RAND_LEN) == 0) {
      temp = snprintf(write_ptr, free_space, "\"%ld\",",
                      (random() % DEFAULT_RAND_MOD) + 1);
      pos += PLACEHOLDER_RAND_LEN;
    } else if (strncmp(pos, PLACEHOLDER_SEQ, PLACEHOLDER_SEQ_LEN) == 0) {
      temp = snprintf(write_ptr, free_space, "\"%d\",", next_id(worker, i));
      pos += PLACEHOLDER_SEQ_LEN;
    } else {
      return 0;
    }

    write_ptr += temp;
    query_length += temp;
    free_space -= temp;
  }

  /* rewind 1 byte to ignore the final comma */
  write_ptr--;

  /* copy everything up to the end of the statement */
  strncpy(write_ptr, pos, buflen-query_length);
  return query_length;
}

bool preload_sql_file(SKY_SHARE *share) {
  assert(share && share->sql_file_path && !share->query_list);
  FILE *sql_file;
  char buffer[SKY_STRSIZ];

  /* open the SQL file */
  if (!(sql_file = fopen(share->sql_file_path, "r"))) {
    report_error("failed to open specified SQL file");
    return false;
  }  

  /* create a linked list to load the content to memory */
  if ((share->query_list = sky_list_new()) == NULL) {
    fclose(sql_file);
    return false;
  }

  /* push all queries found to the list */
  while (fgets(buffer, SKY_STRSIZ, sql_file) != NULL) {
    if (!sky_list_push(share->query_list, buffer, strlen(buffer))) {
      sky_list_free(share->query_list);
      fclose(sql_file);
      return false;
    }
  }

  fclose(sql_file);
  return true;
}
