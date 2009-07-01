/* 
 * Copyright (C) 2009 Toru Maesaka <dev@torum.net>
 * All Rights Reserved.
 *
 * Use and distribution of this program is licensed under the
 * BSD license. See the COPYING file for full text.
 */

#include "skyfall_generator.h"

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

size_t get_next_insert_query(SKYFALL_WORKER *worker, char *buffer,
                             size_t buflen) {
  return 0;
}
