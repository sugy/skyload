/* 
 * Copyright (C) 2009 Toru Maesaka <dev@torum.net>
 * All Rights Reserved.
 *
 * Use and distribution of this program is licensed under the
 * BSD license. See the COPYING file for full text.
 */

#include "skyfall_generator.h"

static uint32_t next_id(SKYFALL_WORKER *worker, uint32_t col_num) {
  assert(worker);
  uint32_t delta = worker->unique_id + worker->share->concurrency;
  return worker->current_seq_id[col_num] += delta;
}

SKYFALL_COLUMN_LIST *analyze_insert_template(SKYFALL_SHARE *share) {
  assert(share && share->insert_tmpl);

  SKYFALL_COLUMN_LIST *list;
  SKYFALL_COLUMN_TYPE type;
  char *pos;

  if ((list = skyfall_column_list_new()) == NULL)
    return NULL;

  /* calculate the number of columns we need to work with */
  share->columns = string_occurrence(share->insert_tmpl, "$");

  if (share->columns == 0) {
    report_error("INSERT template doesn't contain any placeholders");
    return NULL;
  }

  pos = share->insert_tmpl;

  for (int i = 0; i < share->columns; i++) {
    pos = strchr(pos, '$');

    if (strncmp(pos, PLACEHOLDER_RAND, PLACEHOLDER_RAND_LEN) == 0) {
      type = COLUMN_RANDOM;
    } else if (strncmp(pos, PLACEHOLDER_SEQ, PLACEHOLDER_SEQ_LEN) == 0) {
      type = COLUMN_SEQUENTIAL;
    } else {
      skyfall_column_list_free(list);
      return NULL;
    }

    skyfall_column_list_push(list, type, 8);
    pos++;
  }
  return list;
}

size_t next_insert_query(SKYFALL_WORKER *worker, char *buffer,
                         size_t buflen) {
  size_t query_length;  
  char *pos, *write_ptr;

  /* copy everything up to the values. e.g. 'INSERT INTO t1 VALUES (' */
  write_ptr = buffer;
  pos = strchr(worker->share->insert_tmpl, '$');
  query_length = 0;

  if (pos == NULL)
    return 0;

  size_t temp = pos - worker->share->insert_tmpl;
  size_t free_space = buflen - temp;

  if (temp > SKYFALL_STRSIZ) {
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
    pos = strchr(pos, '$');

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
